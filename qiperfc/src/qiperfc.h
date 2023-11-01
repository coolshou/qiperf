#ifndef QIPERFC_H
#define QIPERFC_H

#include <QMainWindow>
#include <QLabel>
#include <QItemSelection>
#include <QMap>
#include <QList>
#include <QTimer>
#include <QPen>

#include "pipeclient.h"
#include "jcon/json_rpc_websocket_client.h"
#include "dlgiperf.h"
#include "udpreceiver.h"
//#include "tpchart.h"
#include "endpointmgr.h"
#include "formendpoints.h"
#include "tpmgr.h"
#include "tpdirdelegate.h"

#define TEST_JSONRPC 0
#define TEST_PLOT_DATA 0

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
    int createRPC_Server(QString host="127.0.0.1", int port=RPC_PORT);
    int createRPC_Client(QString host="127.0.0.1", int port=RPC_PORT);
    void notificationReceived(const QString key, const QVariant value);

signals:
    void updateEndpointNum(int n);

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

private:
    Ui::MainWindow *ui;
    DlgIperf * dlgiperf;  // dialog of iperf config
    FormEndPoints * formEndpoits;
    PipeClient *pclient;
#if (TEST_JSONRPC==1)
    jcon::JsonRpcWebSocketClient *rpc_client;
#endif
    QMap<QString, jcon::JsonRpcWebSocketClient *> map_qiperfds_server; // manager all qiperfd <manager ip, rpc_client> for iperf server
    QMap<QString, jcon::JsonRpcWebSocketClient *> map_qiperfds_client; // manager all qiperfd <manager ip, rpc_client> for iperf client
    UdpReceiver *m_receiver;
//    QChartView *m_tpchart;
//    TPChart *m_tpchart;
    EndPointMgr *m_endpointmgr;
    QLabel *m_endpoint_label;
    TPMgr *m_tpmgr;
    TPDirDelegate *tpdrdelegate;
    QTimer dataTimer;

};
#endif // QIPERFC_H
