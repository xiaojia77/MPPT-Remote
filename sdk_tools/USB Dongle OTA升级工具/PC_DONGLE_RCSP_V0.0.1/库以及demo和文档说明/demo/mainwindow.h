#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../libdongle/libdongle.h"
#include "taskthread.h"
#include <memory>
#include <QByteArray>
#include <QListWidget>
#include <QMainWindow>
#include <QProgressBar>
#include <QAbstractNativeEventFilter>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace libdongle {
class Executor;
}

struct DevInfo
{
    QString name;
    QString path;

    DevInfo(const QString &n, const QString &p) : name(n), path(p) {}
};

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void checkDeviceList();
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
    void updateDeviceList();

    bool stopCurrentTaskThread();

    void onOtaStart();
    void onOtaStartChannel(int channel);

    void onOtaFinished();
    void onOtaFinishedChannel(int channel);

    void onOtaError(int code, const QString &msg);
    void onOtaErrorChannel(int channel, int code, const QString &msg);

    void onOtaProgress(int step, int total);
    void onOtaProgressChannel(int channel, int step, int total);

    void onDongleInfo(uint32_t sdk_version, uint16_t vid, uint16_t pid, uint16_t pidVersion);

    void onRefreshClientInfo(uint32_t client_count, struct dongle_client_info_t *client_infos);

    ota_upgrade_callback_t getOtaCallback();
    dongle_callback_t getDongleCallback();
    ota_upgrade_callback_t getOtaCallback(int channel);

private slots:
    void on_btnOpen_clicked();

    void on_btnReadDongleInfo_clicked();

    void on_btnClose_clicked();

    void on_btnRefresh_clicked();

    void on_btnUpdateDongle_clicked();

    void on_bntUpdateDongleSignlePost_clicked();

    void on_btnUpdateClient_clicked();

    void on_btnSelectUpgradeFile_clicked();

    void on_btnListRemoteClients_clicked();

    void on_btnUpdateDongleAll_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_btnBatchUpdate_clicked();

    //void on_listWidget_itemSelectionChanged();
    void on_contextMenuRequested(const QPoint &pos);
    void setDeviceUnselect();
    void on_checkBoxAutoUpdate_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<libdongle::Executor> m_ex;
    TaskThread *m_taskThread{nullptr};
    std::shared_ptr<QByteArray> m_upgradeContent;
    std::vector<dongle_client_info_t> m_clientInfos;
    qint64 m_updateStartTime;
    std::vector<QProgressBar *> m_progressBars;
    std::vector<qint64> m_updateStartTimes;
    std::vector<std::shared_ptr<QByteArray>> m_upgradeContents;
    QTimer *m_refreshTimer;
    QString m_ufwTypeString;
    std::shared_ptr<QByteArray> m_currentUpgradeContent;
    QAction *m_actUnselect;
    bool m_newDevice;
    bool m_newDongle;
};
#endif // MAINWINDOW_H
