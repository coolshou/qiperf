#ifndef QIPERFC_H
#define QIPERFC_H

#include <QMainWindow>
#include <QLabel>
#include <QItemSelection>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QPen>
#include <QObject>
#include <QSettings>
#include <QClipboard>
#include <QMenu>
#include <QDialog>

#include "comm.h"
#include "udpreceiver.h"
#include "endpointmgr.h"
#include "qipconfig.h"
#include "formqiperfds.h"
#include "dlgtest.h"
#include "dlgoption.h"
#include "dlgrecord.h"
#include "dlgserial.h"

// #include "customheaderview.h"
#include "fileserver.h"
#include "pingmgr.h"
#include "pingplot.h"
#include "../../src/dlgshowlog.h"
#include "exporthtml.h"
#include "../views/viewmanager.h"
#include "../views/throughputview.h"
#include "../views/serialview.h"

#if (TEST_ICMP==1)
#include "../src/icmpping.h"
#endif
#if (TEST_WS==1)
#include "wsclient.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class QIperfC : public QMainWindow
{
    Q_OBJECT

public:
    QIperfC(QString logpath, QWidget *parent = nullptr);
    ~QIperfC() override;
    void start();
    void stop();
    bool load(QString filename); // load tp config
    bool save(QString filename); // save tp config
    QString getNowString();

public slots:
    void onNewMessage(const QString msg);
    void onNew();
    void onOpen();
    void onSave();
    void onImportIperf3Log();
    void onImportIperf2Log();
    bool on_Clear(bool showNotice=true);
    void initStart();
    void onStart();
    void onStop();
    bool onClear(bool showNotice=true);
    void onShowLog();
    void onConfig();
    void onAbout();
    void onShowDebugLog();

    void onErrorStop(int err, QString msg);
    void onNotice(QString send_addr, QString msg);
    void onQuit();
    void notificationReceived(const QString key, const QVariant value);
    void setStartTime(QDateTime startTime);
    void setShowGroup(bool bShow);
    void onUpdateTPUnit(QString sunit);
    //test
    void onTest();
    void onTestStarted();
    void onTestStoped(int err);

signals:
    void updateEndpointNum(int n);
    void updateStarttime(QString stime);
    void updateStatus(QString msg);
    void errorStop(int err, QString msg); // signal when test error
    void testStarted(); // signal when test started
    void testStoped(int err); // signal when test stoped, 0: no error
    void closeAll(); // send signal to close all dialog
    void setEndTime(double value);
    void updateInterval(int interval);

protected:
    void closeEvent(QCloseEvent *event)override;
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    void updateRunStatus(bool bStart);

    // void initPingChart();
    void resetError();
    void saveSettings();
    void loadSettings();
    void doClear();

private slots:
    void initActions();
    void initToolbar();
    void initStatusbar();
    void onUpdateStarttime(QString stime);
    void onUpdateStatus(QString msg);
    void onUpdateActions(bool bStart, bool bStop, bool bClear);
    void onUpdateActionsSave(bool bSave);
    void onUpdateActionsEdit(bool bDel, bool bEdit, bool bSwap, bool bSwapIP);
    void on_updateQIperfdNum(int n);

    void onRPC_result(const QVariant& result);
    void onRPC_error(int code, const QString& message);
    void onIperfStarted(QString smode, QString ipport);
    void onIperfStoped(QString refrow, QString err_no, QString err, QString ipport);
    void onDisconnected(QString targetip);


    void onUpdateDataPath(QString datapath);

    void onProgress(QString filename, int currentlineno);
    int getStatusServers();
    int getStatusClients();
    void onAddSerial();
    void onAddPing();
    void onWlanSTA();
    void onError(QString msg);
    void onFileServerError(QString msg);
    void onExport();
    void onWidthChanged(int width);
    void onHeigthChanged(int heigth);
    void onShowGroup(bool bShow);
    void onIgnoreWrongInterval(bool bIgnore);
    void onSerialOpened(QString refrow, QString serveraddress, QString serveraPort);
private:
    Ui::MainWindow *ui;
    // CustomHeaderView *header;
    QString settingfilepath;
    QString m_logpath;
    QClipboard *m_clipboard;
    FormQIperfds *m_frm_qiperfds;
    dlgOption *m_frm_option;
    DlgTest *m_dlgtest;
    DlgRecord *m_dlgrecord; //TODO: store final test result
    DlgSerial *m_dlgserial; // serial select dialog
    PingPlot *m_pingplot;
    QSettings *m_settings;
    bool m_testping;
    int iExtraWait = 5;
//    PipeClient *pclient;
#if (TEST_WS==1)
    QMap<QString, WSClient *> m_wss; // websocket client list for manager iperf server
    QMap<QString, WSClient *> m_wsc; // websocket client list for manager iperf client
    QMap<QString, int> m_status_server; // store server status, 0: init, 1: running, 2: error?
    QMap<QString, int> m_status_client; // store client status, 0: init, 1: running, 2: error?
    WSClient *ws;
#endif
    UdpReceiver *m_receiver;
    EndPointMgr *m_endpointmgr;
    QLabel *m_start_label;
    QLabel *m_status_label;
    QLabel *m_label_qiperfd;

    QDateTime m_TestStartTime;
    bool bStartTest;
    int bErrorStop;
    bool bUserStop;
    int iTimeout; // default wait websocket timeout 10
    QString m_ErrorMSG;
    QString m_tpcfgname; //tp config file name
    QIPConfig *m_qipconfig;
    int m_WaitServerReady;
    int m_TPExportWidth;
    int m_TPExportHeigth;
    bool m_TPGroup; // show throughput group
    QString m_TPUnit; // store throughput format unit,
    QString m_datapath;

    FileServer *m_fileserver;
    QString m_oldsavepath=nullptr;

    DlgShowLog *m_dlgshowlog;
#if (TEST_ICMP==1)
    //TEST icmp
    IcmpPing *m_icmpping;
    DlgPing *dp;
#endif
    PingMgr *m_pingmgr;
    QDialog *m_debugdlg;
    ExportHtml *eh;
    ThroughputView *m_throughputview;
    ViewManager *m_views;
    bool m_IgnoreWrongInterval;
    QMap<QString, SerialView*> *m_serialviews; // store serial view
    //log info for serial view
    bool _logtofile;
    QString _logfilename;
    bool _logtimestemp;
    QString _logtimestempformat;
};
#endif // QIPERFC_H
