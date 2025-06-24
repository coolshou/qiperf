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
#include <QThread>

#include "comm.h"
#include "udpreceiver.h"
#include "endpointmgr.h"
#include "qipconfig.h"
#include "formqiperfds.h"
#include "dlgtest.h"
#include "dlgoption.h"
#include "dlgrecord.h"
#include "dlgserial.h"
#include "dlgssh.h"

// #include "customheaderview.h"
#include "fileserver.h"
#include "pingmgr.h"
#include "pingplot.h"
#include "../../src/dlgshowlog.h"
#include "exporthtml.h"
#include "../views/viewmanager.h"
#include "../views/throughputview.h"
#include "../views/serialview.h"
#include "../src/gps/dlggpscalc.h"
#include "../views/serialdata.h"
#include "../views/sshdata.h"
#include "plugin/plugininterface.h"
#include "auto/dlgsimplemicro.h"

#if (TEST_ICMP==1)
#include "../src/icmpping.h"
#endif
#if (TEST_WS==1)
#include "wsclient.h"
#endif
#include "tpworker.h"

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
    // void initStart();
    void onStart(bool showNotice=true);
    void onStop();
    bool onClear(bool showNotice=true);
    void onShowLog();
    void onConfig();
    void onSimpleMicro();
    void onAbout();
    void onShowDebugLog();

    void onErrorStop(int err, QString msg);
    void onDebuginfo(QString msg);
    void onNotice(QString send_addr, QString msg);
    void onQuit();
    void notificationReceived(const QString key, const QVariant value);
    void setStartTime(QDateTime startTime);
    void setShowGroup(bool bShow);
    void onUpdateTPUnit(QString sunit);
    //test
    void onTestStarted();
    void onTestStoped(int err);

    void onCopy();
    void onPaste();
    void onDelete();
    void onCopyText();

signals:
    void updateEndpointNum(int n);
    void updateStarttime(QDateTime stime);
    void updateStatus(QString msg);
    // void errorStop(int err, QString msg); // signal when test error
    void testStarted(); // signal when test started
    void testStoped(int err); // signal when test stoped, 0: no error
    void closeAll(); // send signal to close all dialog
    void setEndTime(double value);
    void updateInterval(int interval);
    void reportTP(int idx, double tp, double lostrate);
    void setTPStop();
    void doNtpSync(QString target);

protected:
    void closeEvent(QCloseEvent *event)override;
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    void loadPlugins();
    void unloadPlugins();
    void updateRunStatus(bool bStart);

    // void initPingChart();
    void resetError();
    void saveSettings();
    void loadSettings();
    void doClear();
    void AddSerialView(QString mkey, SerialView *serialview, WSClient *wsc);
    void AddSSHView(QString mkey, SSHView *sshview, WSClient *wsc);

private slots:
    void initActions();
    void initToolbar();
    void initStatusbar();
    void onUpdateStarttime(QDateTime stime);
    void onUpdateStatus(QString msg);
    void onUpdateRunStatus(bool bStart);
    void onUpdateActions(bool bStart, bool bStop, bool bClear);
    void onUpdateActionsSave(bool bSave);
    void onUpdateActionsEdit(bool bDel, bool bEdit, bool bSwap, bool bSwapIP);
    void on_updateQIperfdNum(int n);
    void onRPC_result(const QVariant& result);
    void onRPC_error(int code, const QString& message);
    void onUpdateDataPath(QString datapath);
    void onProgress(QString filename, int currentlineno);

    void onAddSerial();
    void onAddSSH();
    void onGPScalc();
    void onAddPing();
    void onWlanSTA();
    void onError(QString msg);
    void onFileServerError(QString msg);
    void onExport();
    void onWidthChanged(int width);
    void onHeigthChanged(int heigth);
    void onShowGroup(bool bShow);
    void onIgnoreWrongInterval(bool bIgnore);
    void onUpdateOpenStreetMapTile(QString tile);
    void onSerialOpened(QString refrow, QString serveraddress, QString serveraPort);
    void onSerialClosed(QString idx);
    void onSSHOpened(QString refrow, QString serveraddress, QString serveraPort);
    void onSSHClosed(QString idx);
    void showView();
    void onAutoLoadFile(QString idx, QString filename, QString savepath);
    void onDoNtpSync(QString target);
    void onNtpsynced(bool bOK, QString target);
private:
    Ui::MainWindow *ui;
    QList<QPluginLoader*> pluginLoaders;
    QList<PluginInterface*> plugins;
    // CustomHeaderView *header;
    QString settingfilepath;
    QString m_logpath;
    QClipboard *m_clipboard;
    FormQIperfds *m_frm_qiperfds;
    dlgOption *m_dlgoption;
    DlgTest *m_dlgtest;
    DlgRecord *m_dlgrecord; //TODO: store final test result
    DlgSerial *m_dlgserial; // serial select dialog
    DlgSSH *m_dlgssh; // ssh select dialog
    PingPlot *m_pingplot;
    QSettings *m_settings;
    bool m_testping;
    int iExtraWait = 5;
//    PipeClient *pclient;
#if (TEST_WS==1)
    // WSClient *m_ws;
    QMap<QString, WSClient*> m_ws;
    QList<QString> m_ntps; // collect of synced ntp clients
    int m_maxntpsync=5;
    QMap<QString, int> m_ntpfail; // collesct of sync fail client, try times
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
    QMap<QString, SerialData> *m_serialviews; // store serial view
    QMap<QString, SSHData> *m_sshviews; // store serial view
    //log info for serial view
    bool _logtofile;
    QString _logfilename;
    bool _logtimestemp;
    QString _logtimestempformat;
    DlgGpsCalc *dlg_gps;
    QString m_OpenStreetMapTile;

    // throughput worker
    TpWorker *m_tpworker;
    QThread *m_tpthread;
    //automate: simple micro
    int m_smicroIdx;
    DlgSimpleMicro *smicro;
};
#endif // QIPERFC_H
