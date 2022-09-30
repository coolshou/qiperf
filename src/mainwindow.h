#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QProcess>
#include <QSettings>

#include "iperfworker.h"
#include "formoption.h"
#if !defined (Q_OS_ANDROID)
#include "formconsole.h"
#endif
#include "libmaia/maia/maiaXmlRpcServer.h"
#include "agent.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    bool isBigScreen();
    void onReceivedMessage(QString message);

private slots:
    void on_pb_run_clicked();
    void readStdOut(QString text);
    void readStdErr(QString text);
    void onStarted();
    void onFinished(int exitCode, int exitStatus);
    void onLog(QString text);

    void on_pb_clear_clicked();

    void on_pb_quit_clicked();

    void on_pb_copy_clicked();

    void on_rb_v2_clicked();

    void on_rb_v3_clicked();

    void on_rb_v21_clicked();

    void on_pb_cfg_clicked();
    void onShowCfg();
    void onShowConsole();
    void onShowHelp();

    void on_cb_client_stateChanged(int arg1);
    void update_ipaddrs();

private:
    void loadcfg();
    Agent *m_agent;
    QString m_ip;
    int m_port;
//#if defined (Q_OS_ANDROID)
#if defined (Q_OS_LINUX)
    QString m_path;
#endif
    QString m_iperfexe2;
    QString m_iperfexe3;
    QStringList m_interfaces;

    Ui::MainWindow *ui;
    QThread *iperf_th;
    IperfWorker *iperfer;
//    FormOption *option;
    QSettings *cfg;
    QMenu *menucfg;
    QAction *actCfg;
    QAction *actConsole;
    QAction *actHelp;
    FormOption *option;
#if !defined (Q_OS_ANDROID)
    FormConsole *console;
#endif
    MaiaXmlRpcServer *m_server;
};
#endif // MAINWINDOW_H
