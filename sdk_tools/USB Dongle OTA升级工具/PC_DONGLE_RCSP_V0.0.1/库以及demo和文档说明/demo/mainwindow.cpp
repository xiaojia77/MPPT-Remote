#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <Dbt.h>
#include <Windows.h>
#include <hidsdi.h>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTimer>

#include "../libdongle/executor.h"
#include "../libdongle/tasks.h"
#include "../libdongle/usbdevice.h"
#include "../libdongle/utils.h"

#define VERSION "V0.0.1"
#define DATE "20221109.1"

#define FAIL_STYLE \
    "QProgressBar { text-align: center; } QProgressBar::chunk { background-color: red; }"
#define SUCC_STYLE \
    "QProgressBar { text-align: center; } QProgressBar::chunk { background-color: green; }"

#define REFRESH_TIME 2000
namespace {

struct channel_info_t
{
    MainWindow *win;
    int channel;
};

void on_start_ota(void *ctx)
{
    auto w = (MainWindow *) ctx;
    w->onOtaStart();
}

void on_start_ota_channel(void *ctx)
{
    auto i = (channel_info_t *) ctx;
    i->win->onOtaStartChannel(i->channel);
}

void on_finish_ota(void *ctx)
{
    auto w = (MainWindow *) ctx;
    w->onOtaFinished();
}

void on_finish_ota_channel(void *ctx)
{
    auto i = (channel_info_t *) ctx;
    i->win->onOtaFinishedChannel(i->channel);
}

void on_progress(void *ctx, int step, int total)
{
    auto w = (MainWindow *) ctx;
    w->onOtaProgress(step, total);
}

void on_progress_channel(void *ctx, int step, int total)
{
    auto i = (channel_info_t *) ctx;
    i->win->onOtaProgressChannel(i->channel, step, total);
}

void on_error(void *ctx, int error, const char *msg)
{
    auto w = (MainWindow *) ctx;
    w->onOtaError(error, msg);
}

void on_error_channel(void *ctx, int error, const char *msg)
{
    auto i = (channel_info_t *) ctx;
    i->win->onOtaErrorChannel(i->channel, error, msg);
}

void on_dongle_info(void *ctx, uint32_t sdk_version, uint16_t vid, uint16_t pid, uint16_t pidVersion)
{
    auto w = (MainWindow *) ctx;
    w->onDongleInfo(sdk_version, vid, pid, pidVersion);
}

void on_refresh_client_info(void *ctx, uint32_t client_count,
                            struct dongle_client_info_t *client_infos)
{
    auto w = (MainWindow *) ctx;
    w->onRefreshClientInfo(client_count, client_infos);
}

void DispatchToMainThread(std::function<void()> callback)
{
    // any thread
    QTimer *timer = new QTimer();
    timer->moveToThread(qApp->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [=]() {
        // main thread
        callback();
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

std::vector<DevInfo> getOnlineDevices(const QString &filters)
{
    auto infos = libdongle::UsbDevice::iterAllHidDevice();

    std::vector<DevInfo> paths;
    for (auto &i : infos) {
        auto name = QString::fromStdString(i.name());
        auto path = QString::fromStdString(i.path());

        qDebug() << "name: " << name << ", " << path;
        /*if (!name.contains("\xe4\xbe\x9b\xe5\xba\x94\xe5\x95\x86\xe5\xae\x9a\xe4\xb9\x89\xe8\xae\xbe\xe5\xa4\x87")) {
            continue;
        }*/

        // "vid_4c4a&pid_4155"
        if (!path.contains(filters)) {
            continue;
        }
        auto devpath = libdongle::toUtf16(i.path());

        HANDLE h = CreateFile(devpath.c_str(), GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0,
                              nullptr);
        if (h == INVALID_HANDLE_VALUE) {
            qDebug() << "open failed";
            continue;
        }
        CloseHandle(h);
        paths.emplace_back(name, path);
    }
    return paths;
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("PC DONGLE RCSP " VERSION " " DATE);
    m_refreshTimer = nullptr;

    ui->btnReadDongleInfo->hide();
    ui->btnClose->hide();
    ui->boxDevice->hide();
    ui->btnRefresh->hide();
    ui->btnOpen->hide();
    ui->btnListRemoteClients->hide();
    ui->boxClients->hide();
    ui->checkBoxParallel->hide();
    ui->btnUpdateClient->hide();
    ui->btnSelectUpgradeFile->hide();

    m_progressBars.push_back(ui->progressBar1);
    m_progressBars.push_back(ui->progressBar2);
    m_progressBars.push_back(ui->progressBar3);
    m_progressBars.push_back(ui->progressBar4);
    for (auto m : m_progressBars) {
        m->hide();
    }

    m_actUnselect = new QAction(tr("unselect"));
    connect(m_actUnselect, &QAction::triggered, this, &MainWindow::setDeviceUnselect);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this,
            &MainWindow::on_contextMenuRequested);

    //updateDeviceList();
    checkDeviceList();
}

MainWindow::~MainWindow()
{
    m_ex = nullptr;
    delete ui;
}
void MainWindow::checkDeviceList()
{
    auto devpaths = getOnlineDevices("vid_4c4a&pid_4155");
    ui->listWidget->clear();
    m_clientInfos.clear();
    m_upgradeContents.clear();
    for (auto &d : devpaths) {
        if (m_taskThread != nullptr) {
            while (m_taskThread->isRunning()) {
                return;
            }
        }

        qDebug() << d.name;
        auto path   = d.path;
        auto usbDev = std::make_shared<libdongle::UsbDevice>();
        if (!usbDev->openAsync(path.toStdString())) {
            ui->logEdit->appendPlainText(tr("open %1 faield").arg(path));
            break;
        }

        m_ex = std::make_shared<libdongle::Executor>(usbDev);
        ui->logEdit->appendPlainText(tr("device %1 opened").arg(path));
        auto cb      = getDongleCallback();
        m_taskThread = new TaskThread([=]() {
            m_ex->submitTask(libdongle::read_online_device(*m_ex, cb));
            m_ex->runAllTask();
        });
        m_taskThread->start();
        while (m_taskThread->isRunning())
            ;
        qDebug() << m_clientInfos.size();
        if (m_clientInfos.size() > 0) {
            if (m_refreshTimer != nullptr) {
                m_refreshTimer->deleteLater();
            }
            m_refreshTimer = new QTimer();
            QObject::connect(m_refreshTimer, &QTimer::timeout, [=]() {
                // main thread
                if (m_taskThread != nullptr) {
                    if (m_taskThread->isRunning()) {
                        return;
                    }
                }
                if (m_ex == nullptr) {
                    m_refreshTimer->stop();
                    return;
                }
                if (ui->checkBoxAutoUpdate->isChecked() == false) {
                    if (ui->btnBatchUpdate->isEnabled() == false) {
                        ui->btnBatchUpdate->setDisabled(false);
                    }
                }
                for (auto m : m_progressBars) {
                    m->hide();
                }
                on_btnListRemoteClients_clicked();
                if (ui->checkBoxAutoUpdate->isChecked()) {
                    if (m_clientInfos.size() <= 0) {
                        return;
                    }
                    if (m_upgradeContent == nullptr) {
                        ui->checkBoxAutoUpdate->setChecked(false);
                        ui->logEdit->appendPlainText(tr("please select ufw file"));
                        QMessageBox::warning(this, tr("please select ufw file"),
                                             tr("please select ufw file"));
                        return;
                    }
                    if (m_newDevice == false) {
                        return;
                    }
                    for (uint32_t i = 0; i < m_clientInfos.size(); ++i) {
                        while (m_taskThread->isRunning())
                            ;
                        if (m_ufwTypeString != m_clientInfos[i].device_name) {
                            continue;
                        }
                        if (ui->listWidget->count() != m_clientInfos.size()) {
                            qDebug() << "list count:" << ui->listWidget->count()
                                     << "  clientInfos:" << m_clientInfos.size();
                            return;
                        }
                        ui->listWidget->item(i)->setForeground(Qt::red);
                        qDebug() << "set red:" << ui->listWidget->item(i)->text() << "i" << i;
                        int index   = i;
                        int channel = m_clientInfos.at(i).channel_seq;
                        if (channel == 2) {
                            // usb dongle

                            return;
                        }
                        if (m_ex) {
                            auto auth1 = ui->boxAuth1->isChecked();
                            auto auth2 = ui->boxAuth2->isChecked();
                            auto cb    = getOtaCallback();

                            if (m_upgradeContent == nullptr) {
                                ui->logEdit->appendPlainText(tr("no upgrade content"));
                                return;
                            }

                            QString macAddr
                                = QByteArray((const char *) m_clientInfos.at(index).mac_addr, 6)
                                      .toHex();
                            qDebug() << QString("try to update %1, channel = %2, mac=%3")
                                            .arg(m_clientInfos[i].device_name)
                                            .arg(channel)
                                            .arg(macAddr);
                            ui->logEdit->appendPlainText(
                                tr("try to update %1, channel = %2, mac=%3")
                                    .arg(m_clientInfos[i].device_name)
                                    .arg(channel)
                                    .arg(macAddr));

                            m_taskThread = new TaskThread([=]() {
                                auto pbuf = m_upgradeContent;

                                std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                                         (const uint8_t *) pbuf->data()
                                                             + pbuf->size());

                                m_ex->submitTask(libdongle::ota_update_client(*m_ex, channel, buf,
                                                                              auth1, auth2, cb));
                                m_ex->runAllTask();
                            });
                            m_taskThread->start();
                        } else {
                            ui->logEdit->appendPlainText(tr("device not opened"));
                            QMessageBox::warning(this, tr("device not opened"),
                                                 tr("device not opened"));
                        }
                        return;
                    }
                }
            });

            m_refreshTimer->start(REFRESH_TIME);

            break;
        }
    }
    return;
}
bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result)
    Q_UNUSED(result)
    //if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
    MSG *pMSG    = reinterpret_cast<MSG *>(message);
    auto msgType = pMSG->message;
    if (msgType == WM_DEVICECHANGE) {
        auto lParam = pMSG->lParam;

        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;

        switch (pMSG->wParam) {
        case DBT_DEVNODES_CHANGED: {
            checkDeviceList();
            break;
        }
        default:
            break;
        }
    }
    //}

    return false;
}

void MainWindow::updateDeviceList()
{
    ui->boxDevice->clear();
    auto devpaths = getOnlineDevices("vid_4c4a&pid_4155");
    for (auto &d : devpaths) {
        ui->boxDevice->addItem(d.name, d.path);
    }
}

bool MainWindow::stopCurrentTaskThread()
{
    if (m_taskThread == nullptr) {
        return true;
    }
    if (m_taskThread->isRunning()) {
        QMessageBox::warning(this, tr("wait current task finished"),
                             tr("please wait current task stopped"));
        return false;
    }
    m_taskThread->deleteLater();
    m_taskThread = nullptr;
    return true;
}

void MainWindow::onOtaStart()
{
    qDebug() << "ota started";
    DispatchToMainThread([=] {
        m_updateStartTime = QDateTime::currentMSecsSinceEpoch();
        ui->progressBar->setStyleSheet(SUCC_STYLE);
        ui->logEdit->appendPlainText(tr("ota started"));
    });
}

void MainWindow::onOtaStartChannel(int channel)
{
    qDebug() << QString("ota started (%1)").arg(channel);
    DispatchToMainThread([=] {
        m_updateStartTimes.at(channel) = QDateTime::currentMSecsSinceEpoch();
        m_progressBars.at(channel)->setStyleSheet(SUCC_STYLE);
        ui->logEdit->appendPlainText(tr("ota started (%1)").arg(channel));
    });
}

void MainWindow::onOtaFinished()
{
    DispatchToMainThread([=] {
        auto now  = QDateTime::currentMSecsSinceEpoch();
        auto diff = (now - m_updateStartTime);
        ui->logEdit->appendPlainText(tr("ota finished, time=%1 ms").arg(diff));
        QMessageBox::information(this, tr("finished"), tr("ota finished"));
    });
    m_newDevice = false;
}

void MainWindow::onOtaFinishedChannel(int channel)
{
    DispatchToMainThread([=] {
        auto now  = QDateTime::currentMSecsSinceEpoch();
        auto diff = (now - m_updateStartTimes.at(channel));
        ui->logEdit->appendPlainText(tr("ota (%2) finished, time=%1 ms").arg(diff).arg(channel));
    });
}

void MainWindow::onOtaError(int code, const QString &msg)
{
    qDebug() << QString("ota error: %1, %2").arg(code).arg(msg);
    DispatchToMainThread([=] {
        ui->logEdit->appendPlainText(tr("ota error: %1, %2").arg(code).arg(msg));
        ui->progressBar->setStyleSheet(FAIL_STYLE);
        QMessageBox::warning(this, tr("error"), tr("ota error: %1, %2").arg(code).arg(msg));
        m_newDevice = false;
    });
}

void MainWindow::onOtaErrorChannel(int channel, int code, const QString &msg)
{
    qDebug() << QString("ota (%3) error: %1, %2").arg(code).arg(msg).arg(channel);
    DispatchToMainThread([=] {
        ui->logEdit->appendPlainText(tr("ota (%3) error: %1, %2").arg(code).arg(msg).arg(channel));
        m_progressBars[channel]->setStyleSheet(FAIL_STYLE);
        QMessageBox::warning(this, tr("error"),
                             tr("ota (%3) error: %1, %2").arg(code).arg(msg).arg(channel));
    });
}

void MainWindow::onOtaProgress(int step, int total)
{
    DispatchToMainThread([=] {
        ui->progressBar->setRange(0, total);
        ui->progressBar->setValue(step);
    });
}

void MainWindow::onOtaProgressChannel(int channel, int step, int total)
{
    DispatchToMainThread([=] {
        m_progressBars[channel]->setRange(0, total);
        m_progressBars[channel]->setValue(step);
    });
}

void MainWindow::onDongleInfo(uint32_t sdk_version, uint16_t vid, uint16_t pid, uint16_t pidVersion)
{
    DispatchToMainThread([=] {
        ui->logEdit->appendPlainText(tr("usb donge: version: %1, vid=%2, pid=%3, pidv=%4")
                                         .arg(sdk_version, 8, 16, QChar('0'))
                                         .arg(vid, 4, 16, QChar('0'))
                                         .arg(pid, 4, 16, QChar('0'))
                                         .arg(pidVersion, 4, 16, QChar('0')));
    });
}

void MainWindow::onRefreshClientInfo(uint32_t client_count, dongle_client_info_t *client_infos)
{
    m_clientInfos.clear();
    for (uint32_t i = 0; i < client_count; ++i) {
        m_clientInfos.push_back(client_infos[i]);
    }
    qDebug() << "client_count:" << client_count;

    DispatchToMainThread([=] {
        ui->boxClients->clear();
        m_updateStartTimes.clear();

        for (uint32_t i = 0; i < ui->listWidget->count(); i++) {
            uint32_t count = 0;
            for (uint32_t j = 0; j < m_clientInfos.size(); ++j) {
                auto name = QString(m_clientInfos[j].device_name);
                if ((ui->listWidget->item(i)->data(Qt::UserRole) == m_clientInfos[j].channel_seq)
                    && (ui->listWidget->item(i)->text() == name)) {
                    break;
                }
                count++;
            }
            if (count == m_clientInfos.size()) {
                ui->listWidget->takeItem(i);
                auto iter = m_upgradeContents.begin();
                iter += i;
                m_upgradeContents.erase(iter);
            }
        }
        for (uint32_t i = 0; i < m_clientInfos.size(); ++i) {
            auto name = QString(m_clientInfos[i].device_name);
            ui->boxClients->addItem(name);
            auto item = new QListWidgetItem(name);
            item->setData(Qt::UserRole, m_clientInfos[i].channel_seq);

            auto findslist = ui->listWidget->findItems(name, Qt::MatchFixedString);
            if (findslist.isEmpty()) {
                auto iter = m_upgradeContents.begin();
                iter += i;
                ui->listWidget->insertItem(i, item);
                m_upgradeContents.insert(iter, nullptr);
                m_newDevice = true;
            } else {
                int count = 0;
                for (auto &fi : findslist) {
                    if (fi->data(Qt::UserRole) == m_clientInfos[i].channel_seq) {
                        break;
                    }
                    count++;
                }
                if (count == findslist.count()) {
                    auto iter = m_upgradeContents.begin();
                    iter += i;
                    ui->listWidget->insertItem(i, item);
                    m_upgradeContents.insert(iter, nullptr);
                    m_newDevice = true;
                } else if (ui->listWidget->item(i)->data(Qt::UserRole)
                           != m_clientInfos[i].channel_seq) {
                    for (auto &fi : findslist) {
                        if (fi->data(Qt::UserRole) == m_clientInfos[i].channel_seq) {
                            auto tempItem           = fi;
                            auto row                = ui->listWidget->row(tempItem);
                            auto tempUpgradeContent = m_upgradeContents.at(row);
                            ui->listWidget->takeItem(row);
                            ui->listWidget->insertItem(row, ui->listWidget->item(i));
                            m_upgradeContents.at(row) = m_upgradeContents.at(i);
                            ui->listWidget->takeItem(i);
                            ui->listWidget->insertItem(i, tempItem);
                            m_upgradeContents.at(i) = tempUpgradeContent;
                        }
                    }
                }
            }
            m_updateStartTimes.push_back(0);
        }
    });
}

ota_upgrade_callback_t MainWindow::getOtaCallback()
{
    ota_upgrade_callback_t cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx           = this;
    cb.on_error      = on_error;
    cb.on_progress   = on_progress;
    cb.on_start_ota  = on_start_ota;
    cb.on_finish_ota = on_finish_ota;
    return cb;
}

ota_upgrade_callback_t MainWindow::getOtaCallback(int channel)
{
    ota_upgrade_callback_t cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx           = new channel_info_t{this, channel};
    cb.on_error      = on_error_channel;
    cb.on_progress   = on_progress_channel;
    cb.on_start_ota  = on_start_ota_channel;
    cb.on_finish_ota = on_finish_ota_channel;
    return cb;
}

dongle_callback_t MainWindow::getDongleCallback()
{
    dongle_callback_t cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx                    = this;
    cb.on_error               = on_error;
    cb.on_dongle_info         = on_dongle_info;
    cb.on_refresh_client_info = on_refresh_client_info;
    return cb;
}
void MainWindow::on_btnOpen_clicked()
{
    auto index = ui->boxDevice->currentIndex();
    if (index == -1) {
        ui->logEdit->appendPlainText(tr("no device"));
        return;
    }
    if (!stopCurrentTaskThread()) {
        ui->logEdit->appendPlainText(tr("current task not stopped"));
        return;
    }

    auto path   = ui->boxDevice->currentData().toString();
    auto usbDev = std::make_shared<libdongle::UsbDevice>();
    if (!usbDev->openAsync(path.toStdString())) {
        ui->logEdit->appendPlainText(tr("open %1 faield").arg(path));
        return;
    }
    m_ex = std::make_shared<libdongle::Executor>(usbDev);
    ui->logEdit->appendPlainText(tr("device %1 opened").arg(path));
}

void MainWindow::on_btnReadDongleInfo_clicked() {}

void MainWindow::on_btnClose_clicked()
{
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }
        m_ex = nullptr;
    }
}

void MainWindow::on_btnRefresh_clicked()
{
    updateDeviceList();
}

void MainWindow::on_btnUpdateDongle_clicked()
{
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }

        auto auth1 = ui->boxAuth1->isChecked();
        auto auth2 = ui->boxAuth2->isChecked();
        auto cb    = getOtaCallback();

        if (m_upgradeContent == nullptr) {
            ui->logEdit->appendPlainText(tr("no upgrade content"));
            return;
        }

        m_taskThread = new TaskThread([=]() {
            auto pbuf = m_upgradeContent;

            std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                     (const uint8_t *) pbuf->data() + pbuf->size());

            bool needPost = false;
            m_ex->submitTask(libdongle::ota_update_dongle(*m_ex, 2, buf, ui->boxAuth1->isChecked(),
                                                          ui->boxAuth2->isChecked(), cb, needPost));
            m_ex->runAllTask();
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
}

void MainWindow::on_bntUpdateDongleSignlePost_clicked()
{
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }

        auto auth1 = ui->boxAuth1->isChecked();
        auto auth2 = ui->boxAuth2->isChecked();
        auto cb    = getOtaCallback();

        if (m_upgradeContent == nullptr) {
            ui->logEdit->appendPlainText(tr("no upgrade content"));
            return;
        }

        m_taskThread = new TaskThread([=]() {
            auto pbuf = m_upgradeContent;

            std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                     (const uint8_t *) pbuf->data() + pbuf->size());

            m_ex->submitTask(libdongle::ota_update_dongle_single_post(*m_ex, 2, buf,
                                                                      ui->boxAuth2->isChecked(),
                                                                      cb));
            m_ex->runAllTask();
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
}

void MainWindow::on_btnUpdateClient_clicked()
{
    auto index = ui->boxClients->currentIndex();
    if (index < 0 || index >= m_clientInfos.size()) {
        ui->logEdit->appendPlainText(tr("no client"));
        return;
    }
    int channel = m_clientInfos.at(index).channel_seq;

    if (channel == 2) {
        // usb dongle
        on_btnUpdateDongleAll_clicked();
        return;
    }

    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }

        auto auth1 = ui->boxAuth1->isChecked();
        auto auth2 = ui->boxAuth2->isChecked();
        auto cb    = getOtaCallback();

        if (m_upgradeContent == nullptr) {
            ui->logEdit->appendPlainText(tr("no upgrade content"));
            return;
        }

        QString macAddr = QByteArray((const char *) m_clientInfos.at(index).mac_addr, 6).toHex();
        ui->logEdit->appendPlainText(tr("try to update %1, channel = %2, mac=%3")
                                         .arg(ui->boxClients->currentText())
                                         .arg(channel)
                                         .arg(macAddr));

        m_taskThread = new TaskThread([=]() {
            auto pbuf = m_upgradeContent;

            std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                     (const uint8_t *) pbuf->data() + pbuf->size());

            m_ex->submitTask(libdongle::ota_update_client(*m_ex, channel, buf, auth1, auth2, cb));
            m_ex->submitTask(
                libdongle::read_online_device(*m_ex, getDongleCallback())); // refresh clients
            m_ex->runAllTask();
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
}

void MainWindow::on_btnSelectUpgradeFile_clicked()
{
    auto filePath = QFileDialog::getOpenFileName(this, tr("select upgrade file"), ".",
                                                 tr("ufw file (*.ufw)"));
    if (filePath.isEmpty()) {
        return;
    }
    QFile fin(filePath);
    if (!fin.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("open failed"),
                             tr("failed to open file %1, error %2")
                                 .arg(filePath)
                                 .arg(fin.errorString()));
        return;
    }
    ui->logEdit->appendPlainText(tr("open file: %1").arg(filePath));
    m_upgradeContent = std::make_shared<QByteArray>(fin.readAll());
}

void MainWindow::on_btnListRemoteClients_clicked()
{
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }
        auto cb      = getDongleCallback();
        m_taskThread = new TaskThread([=]() {
            m_ex->submitTask(libdongle::read_online_device(*m_ex, cb));
            m_ex->runAllTask();
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
}

void MainWindow::on_btnUpdateDongleAll_clicked()
{
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            return;
        }

        auto auth1 = ui->boxAuth1->isChecked();
        auto auth2 = ui->boxAuth2->isChecked();
        auto cb    = getOtaCallback();

        if (m_upgradeContent == nullptr) {
            ui->logEdit->appendPlainText(tr("no upgrade content"));
            return;
        }

        m_taskThread = new TaskThread([=]() {
            auto pbuf = m_upgradeContent;

            std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                     (const uint8_t *) pbuf->data() + pbuf->size());

            bool needPost = false;
            m_ex->submitTask(
                libdongle::ota_update_dongle(*m_ex, 2, buf, auth1, auth2, cb, needPost));
            m_ex->runAllTask();

            if (needPost) {
                for (int i = 0; i < 4; ++i) {
                    auto devices = getOnlineDevices("vid_4d4a");
                    if (devices.empty()) {
                        QThread::msleep(3000); // wait for a while (device online)
                        continue;
                    }
                    break;
                }
                auto devices = getOnlineDevices("vid_4d4a");
                if (devices.empty()) {
                    if (cb.on_error) {
                        cb.on_error(cb.ctx, OTA_ERR_client_err, "cannot find devices");
                    }
                    return;
                }
                if (devices.size() != 1) {
                    if (cb.on_error) {
                        cb.on_error(cb.ctx, OTA_ERR_client_err,
                                    "too many devices, maybe failed to enter uboot");
                    }
                    return;
                }
                auto d      = devices.at(0);
                auto usbDev = std::make_shared<libdongle::UsbDevice>();
                if (!usbDev->openAsync(d.path.toStdString())) {
                    cb.on_error(cb.ctx, OTA_ERR_client_err,
                                tr("open %1 failed").arg(d.path).toStdString().c_str());
                    return;
                }
                m_ex = std::make_shared<libdongle::Executor>(usbDev);
                DispatchToMainThread(
                    [=]() { ui->logEdit->appendPlainText(tr("device %1 opened").arg(d.path)); });

                m_ex->submitTask(libdongle::ota_update_dongle_single_post(*m_ex, 2, buf, auth2, cb));
                m_ex->runAllTask();

                //m_ex = nullptr;

                DispatchToMainThread([=]() {
                    ui->logEdit->appendPlainText(tr("device closed"));
                    ui->btnBatchUpdate->setDisabled(false);
                });
            } else {
                m_ex->submitTask(
                    libdongle::read_online_device(*m_ex, getDongleCallback())); // refresh clients
                m_ex->runAllTask();
            }
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    m_refreshTimer->stop();
    auto filePath = QFileDialog::getOpenFileName(this, tr("select upgrade file"), ".",
                                                 tr("ufw file (*.ufw)"));
    if (filePath.isEmpty()) {
        m_refreshTimer->start();
        return;
    }
    QFile fin(filePath);
    if (!fin.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("open failed"),
                             tr("failed to open file %1, error %2")
                                 .arg(filePath)
                                 .arg(fin.errorString()));
        m_refreshTimer->start();
        return;
    }
    ui->logEdit->appendPlainText(tr("open file: %1").arg(filePath));
    ui->listWidget->setCurrentItem(item);
    int row = ui->listWidget->currentRow();

    m_upgradeContents.at(row) = std::make_shared<QByteArray>(fin.readAll());
    m_progressBars.at(row)->setValue(0);
    m_ufwTypeString  = ui->listWidget->currentItem()->text();
    m_upgradeContent = m_upgradeContents.at(row);

    item->setForeground(Qt::red);
    m_refreshTimer->start(REFRESH_TIME);
}

void MainWindow::on_btnBatchUpdate_clicked()
{
    m_refreshTimer->stop();
    ui->btnBatchUpdate->setDisabled(true);
    ui->progressBar->setValue(0);
    auto index = ui->listWidget->currentRow();
    if (index >= 0) {
        if (index >= m_clientInfos.size()) {
            ui->logEdit->appendPlainText(tr("no client"));
            ui->btnBatchUpdate->setDisabled(false);
            m_refreshTimer->start(REFRESH_TIME);
            return;
        }
        int channel = m_clientInfos.at(index).channel_seq;
        qDebug() << "channel=" << channel << " index=" << index;
        if (m_upgradeContents.at(index) == nullptr) {
            if (m_clientInfos.at(index).device_name != m_ufwTypeString) {
                ui->logEdit->appendPlainText(tr("ufw don't match, please select ufw"));
                ui->btnBatchUpdate->setDisabled(false);
                m_refreshTimer->start(REFRESH_TIME);
                return;
            }
        } else {
            m_upgradeContent = m_upgradeContents.at(index);
            m_ufwTypeString  = m_clientInfos.at(index).device_name;
        }
        if (channel == 2) {
            // usb dongle
            on_btnUpdateDongleAll_clicked();
            m_refreshTimer->start(REFRESH_TIME);
            return;
        }
        if (m_ex) {
            if (!stopCurrentTaskThread()) {
                m_refreshTimer->start(REFRESH_TIME);
                ui->btnBatchUpdate->setDisabled(false);
                return;
            }

            auto auth1 = ui->boxAuth1->isChecked();
            auto auth2 = ui->boxAuth2->isChecked();
            auto cb    = getOtaCallback();

            if (m_upgradeContent == nullptr) {
                ui->logEdit->appendPlainText(tr("no upgrade content"));
                ui->btnBatchUpdate->setDisabled(false);
                m_refreshTimer->start(REFRESH_TIME);
                return;
            }

            QString macAddr = QByteArray((const char *) m_clientInfos.at(index).mac_addr, 6).toHex();
            ui->logEdit->appendPlainText(tr("try to update %1, channel = %2, mac=%3")
                                             .arg(ui->listWidget->currentItem()->text())
                                             .arg(channel)
                                             .arg(macAddr));

            m_taskThread = new TaskThread([=]() {
                auto pbuf = m_upgradeContent;

                std::vector<uint8_t> buf((const uint8_t *) pbuf->data(),
                                         (const uint8_t *) pbuf->data() + pbuf->size());

                m_ex->submitTask(
                    libdongle::ota_update_client(*m_ex, channel, buf, auth1, auth2, cb));
                m_ex->runAllTask();
            });
            m_taskThread->start();
        } else {
            ui->logEdit->appendPlainText(tr("device not opened"));
            QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
        }
        m_refreshTimer->start(REFRESH_TIME);
        return;
    }
    if (m_ex) {
        if (!stopCurrentTaskThread()) {
            ui->btnBatchUpdate->setDisabled(false);
            m_refreshTimer->start(REFRESH_TIME);
            return;
        }

        auto auth1 = ui->boxAuth1->isChecked();
        auto auth2 = ui->boxAuth2->isChecked();

        struct info_t
        {
            std::shared_ptr<QByteArray> m_content;
            int channel;
            size_t index;
        };

        std::vector<info_t> remoteInfos;

        info_t dongleInfo;

        for (size_t i = 0; i < m_upgradeContents.size(); ++i) {
            auto content = m_upgradeContents.at(i);
            if (content == nullptr) {
                continue;
            }
            m_progressBars.at(i)->show();
            auto &info = m_clientInfos.at(i);
            if (info.channel_seq != 2) { // dongle
                remoteInfos.push_back({content, info.channel_seq, i});
            } else {
                dongleInfo.m_content = content;
                dongleInfo.channel   = info.channel_seq;
                dongleInfo.index     = i;
            }
        }

        if (remoteInfos.size() == 0 && dongleInfo.m_content == nullptr) {
            // nothing to be update
            ui->btnBatchUpdate->setDisabled(false);
            m_refreshTimer->start(REFRESH_TIME);
            return;
        }
        // 1. upgrade all remote clients
        bool isParallel = ui->checkBoxParallel->isChecked();
        m_taskThread    = new TaskThread([=]() {
            if (isParallel) {
                if (remoteInfos.size() != 0) {
                    std::vector<channel_info_t *> infos;
                    for (auto &i : remoteInfos) {
                        std::vector<uint8_t> buf((const uint8_t *) i.m_content->data(),
                                                 (const uint8_t *) i.m_content->data()
                                                     + i.m_content->size());
                        auto cb = getOtaCallback(i.index);
                        infos.push_back((channel_info_t *) cb.ctx);
                        m_ex->submitTask(
                            libdongle::ota_update_client(*m_ex, i.channel, buf, auth1, auth2, cb));
                    }

                    m_ex->runAllTask();
                    for (auto p : infos) {
                        delete p;
                    }
                }

            } else {
                for (auto &i : remoteInfos) {
                    std::vector<uint8_t> buf((const uint8_t *) i.m_content->data(),
                                             (const uint8_t *) i.m_content->data()
                                                 + i.m_content->size());
                    auto cb = getOtaCallback(i.index);
                    m_ex->submitTask(
                        libdongle::ota_update_client(*m_ex, i.channel, buf, auth1, auth2, cb));
                    m_ex->runAllTask();

                    delete (channel_info_t *) cb.ctx;
                }
            }

            if (dongleInfo.m_content != nullptr) {
                std::vector<uint8_t> buf((const uint8_t *) dongleInfo.m_content->data(),
                                         (const uint8_t *) dongleInfo.m_content->data()
                                             + dongleInfo.m_content->size());
                bool needPost = false;

                auto cb = getOtaCallback(dongleInfo.index);

                m_ex->submitTask(
                    libdongle::ota_update_dongle(*m_ex, 2, buf, auth1, auth2, cb, needPost));
                m_ex->runAllTask();

                if (needPost) {
                    for (int i = 0; i < 4; ++i) {
                        auto devices = getOnlineDevices("vid_4d4a");
                        if (devices.empty()) {
                            QThread::msleep(3000); // wait for a while (device online)
                            continue;
                        }
                        break;
                    }
                    auto devices = getOnlineDevices("vid_4d4a");
                    if (devices.empty()) {
                        if (cb.on_error) {
                            cb.on_error(cb.ctx, OTA_ERR_client_err, "cannot find devices");
                        }
                        m_refreshTimer->start(REFRESH_TIME);
                        return;
                    }
                    if (devices.size() != 1) {
                        if (cb.on_error) {
                            cb.on_error(cb.ctx, OTA_ERR_client_err,
                                        "too many devices, maybe failed to enter uboot");
                        }
                        m_refreshTimer->start(REFRESH_TIME);
                        return;
                    }

                    auto d      = devices.at(0);
                    auto usbDev = std::make_shared<libdongle::UsbDevice>();
                    if (!usbDev->openAsync(d.path.toStdString())) {
                        cb.on_error(cb.ctx, OTA_ERR_client_err,
                                    tr("open %1 failed").arg(d.path).toStdString().c_str());
                        m_refreshTimer->start(REFRESH_TIME);
                        return;
                    }
                    m_ex = std::make_shared<libdongle::Executor>(usbDev);
                    DispatchToMainThread([=]() {
                        ui->logEdit->appendPlainText(tr("device %1 opened").arg(d.path));
                    });

                    m_ex->submitTask(
                        libdongle::ota_update_dongle_single_post(*m_ex, 2, buf, auth2, cb));
                    m_ex->runAllTask();

                    //m_ex = nullptr;

                    DispatchToMainThread([=]() {
                        ui->btnBatchUpdate->setDisabled(false);
                        ui->logEdit->appendPlainText(tr("device closed"));
                        ui->logEdit->appendPlainText(tr("batch update finished"));
                        QMessageBox::information(this, tr("batch update finished"),
                                                 tr("batch update finished"));
                    });
                }

                delete (channel_info_t *) cb.ctx;
            }
        });
        m_taskThread->start();
    } else {
        ui->logEdit->appendPlainText(tr("device not opened"));
        QMessageBox::warning(this, tr("device not opened"), tr("device not opened"));
    }
    m_refreshTimer->start(REFRESH_TIME);
}

void MainWindow::setDeviceUnselect()
{
    m_refreshTimer->stop();
    ui->listWidget->currentItem()->setSelected(false);
    ui->listWidget->setCurrentRow(-1);
    m_refreshTimer->start(REFRESH_TIME);
}
void MainWindow::on_contextMenuRequested(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction(m_actUnselect);

    if (ui->listWidget->currentRow() != -1) {
        menu.exec(mapToGlobal(pos));
    }
}

void MainWindow::on_checkBoxAutoUpdate_toggled(bool checked)
{
    if (checked) {
        m_newDevice = false;
        ui->btnBatchUpdate->setDisabled(true);
    } else {
        ui->btnBatchUpdate->setDisabled(false);
    }
}
