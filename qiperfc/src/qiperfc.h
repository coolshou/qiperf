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

#include "comm.h"
#include "pipeclient.h"
#include "jcon/json_rpc_websocket_client.h"
#include "dlgiperf.h"
#include "udpreceiver.h"
//#include "tpchart.h"
#include "endpointmgr.h"
#include "formendpoints.h"
#include "tpmgr.h"
#include "tpdirdelegate.h"
#include "QIPConfig.h"
#include "tpplot.h"

#if (TEST_WS==1)
#include "wsclient.h"
#endif
#if (TEST_JSONRPC==1)
#include "rpctp.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QIperfC : public QMainWindow
{
    Q_OBJECT

public:
    QIperfC(QWidget *parent = nullptr);
    ~QIperfC() override;
    void start();
    void stop();
    bool load(QString filename); // load tp config
    bool save(QString filename); // save tp config
    QString getNowString();

public slots:
    void onNewMessage(const QString msg);
    void on_New();
    void on_Open();
    void on_Save();
    void on_Clear();
    void on_pairAdd();
    void on_pairEdit();
    void on_pairDelete();
    void onPairSwap();
    void onStart();
    void onStop();
    void onClear();
    void onAbout();
    void aboutQCustomPlot();
    void onErrorStop(int err, QString msg);
    void on_notice(QString send_addr, QString msg);
    void onQuit();
#if (TEST_JSONRPC==1)
    int createRPC_Server(TP tp, QString host="127.0.0.1", int rpc_port=RPC_PORT);
    int createRPC_Client(TP tp, QString host="127.0.0.1", int rpc_port=RPC_PORT);
#endif
    void notificationReceived(const QString key, const QVariant value);

signals:
    void updateEndpointNum(int n);
    void updateStarttime(QString stime);
    void updateStatus(QString msg);
    void errorStop(int err, QString msg); // signal when test error
    void testStarted(); // signal when test started
    void testStoped(int err); // signal when test stoped, 0: no error

protected:
     void closeEvent(QCloseEvent *event)override;
private:
    void updateRunStatus(bool bStart);
    void initCustomPlote();
//    QPen newColorPen(int r, int g, int b, int width);
    void resetError();
    void saveSettings();
    void loadSettings();

private slots:
    void init_actions();
    void initStatusbar();
    void onUpdateStarttime(QString stime);
    void onUpdateStatus(QString msg);
    void on_pb_status_clicked();
    void on_pb_add_server_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();
    void on_updateEndpointNum(int n);
    void onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTPDataUpdate(const QModelIndex &parent, int first, int last);
    void onItemClicked(QModelIndex idx);
    void onRPC_result(const QVariant& result);
    void onRPC_error(int code, const QString& message);
    void onIperfStarted(QString ipport);
    void onIperfStoped(QString ipport);
    void onIperfTPdata(QString refrow, QString sInterval, QString data);
    void onDisconnected(QString serverip);
    void onPlotContextMenuRequest(QPoint pos);
private:
    Ui::MainWindow *ui;
    TPPlot *m_tpplot;
    QSettings *m_settings;
    DlgIperf * dlgiperf;  // dialog of iperf config
    FormEndPoints * formEndpoits;
    PipeClient *pclient;
#if (TEST_WS==1)
    QMap<QString, WSClient *> m_wss; // websocket client list for manager iperf server
    QMap<QString, WSClient *> m_wsc; // websocket client list for manager iperf client

#endif
#if (TEST_JSONRPC==1)
    jcon::JsonRpcWebSocketClient *rpc_client;
    QMap<QString, RpcTp *> map_qiperfds_server; // manager all qiperfd <manager ip, rpc_client> for iperf server
    QMap<QString, RpcTp *> map_qiperfds_client; // manager all qiperfd <manager ip, rpc_client> for iperf client
#endif
    UdpReceiver *m_receiver;
//    QChartView *m_tpchart;
//    TPChart *m_tpchart;
    EndPointMgr *m_endpointmgr;
    QLabel *m_start_label;
    QLabel *m_status_label;
    QLabel *m_endpoint_label;
    TPMgr *m_tpmgr;
    TPDirDelegate *tpdrdelegate;

    QDateTime m_TestStartTime;
    int bErrorStop;
    int iTimeout; // default wait websocket timeout 10
    QString m_ErrorMSG;
    QString m_tpcfgname; //tp config file name
    QIPConfig *m_qipconfig;

};
#endif // QIPERFC_H
