#include "mainwindow.h"
#include <Windows.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QFile>
#include <QLocale>
#include <QMutex>
#include <QStyleFactory>
#include <QTextStream>
#include <QTranslator>

namespace {

QMutex gLogMutex;
QFile gLogFile;
void InitGlobalLogFile()
{
    QString fileName = QString("demo_log_%1.txt")
                           .arg(QDateTime::currentDateTime().toString("MMddHHmmss.zzz"));
    gLogFile.setFileName(fileName);
    gLogFile.open(QIODevice::ReadWrite | QIODevice::Append);
}

void LogMsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&gLogMutex);
    // cout << msg.toStdString() << endl;
    //Critical Resource of Code
    QByteArray localMsg = msg.toLocal8Bit();
    QString log;

    switch (type) {
    case QtDebugMsg:
        log.append(QString("Debug: %1 %2")
                       .arg(QDateTime::currentDateTime().toString("MM-dd HH:mm:ss.zzz"))
                       .arg(msg));
        break;
    case QtInfoMsg:
        log.append(QString("Info: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtWarningMsg:
        log.append(QString("Warning: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtCriticalMsg:
        log.append(QString("Critical: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        break;
    case QtFatalMsg:
        log.append(QString("Fatal: %1  %2  %3  %4")
                       .arg(localMsg.constData())
                       .arg(context.file)
                       .arg(context.line)
                       .arg(context.function));
        abort();
    }

    QTextStream out(&gLogFile);
    out << log << "\n";
    gLogFile.flush();
}

} // namespace
#define ERR_EXIST       -32
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"{25B4552B-ACE9-4F26-8364-F69435BDA796}");
    if(hMutex != NULL)
    {
        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            exit(ERR_EXIST);
        }
    }
#if 1
    InitGlobalLogFile();
    qInstallMessageHandler(LogMsgOutput);
#endif

    MainWindow w;
    a.installNativeEventFilter(&w);

    w.show();
    return a.exec();
}
