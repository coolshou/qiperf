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

public slots:
    void onNewMessage(const QString msg);
    void on_pairAdd();
    void on_pairEdit();
    void on_pairDelete();
    void onStart();
    void onStop();
    void on_notice(QString send_addr, QString msg);
    void onQuit();
#if (TEST_JSONRPC==1)
    int createRPC_Server(TP tp, QString host="127.0.0.1", int rpc_port=RPC_PORT);
    int createRPC_Client(TP tp, QString host="127.0.0.1", int rpc_port=RPC_PORT);
#endif
    void notificationReceived(const QString key, const QVariant value);

signals:
    void updateEndpointNum(int n);
    void errorStop(int err, QString msg); // signal when test error
    void testStarted(); // signal when test started
    void testStoped(int err); // signal when test stoped, 0: no error

private:
    void updateRunStatus(bool bStart);
    void initCustomPlote();
    void addRandomGraph();
    QPen newColorPen(int r, int g, int b, int width);

private slots:
    void init_actions();
    void initStatusbar();
    void on_pb_status_clicked();
    void on_pb_add_server_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();
//    void On_itemSelectionChanged();
    void on_updateEndpointNum(int n);
    void onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onTPDataUpdate(const QModelIndex &parent, int first, int last);
    void realtimeDataSlot(QPrivateSignal sig);
    void onRPC_result(const QVariant& result);
    void onRPC_error(int code, const QString& message);

private:
    Ui::MainWindow *ui;
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
    QLabel *m_endpoint_label;
    TPMgr *m_tpmgr;
    TPDirDelegate *tpdrdelegate;
    QTimer dataTimer;
    QDateTime m_TestStartTime;

};
#endif // QIPERFC_H
