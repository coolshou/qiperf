#ifndef QIPERFC_H
#define QIPERFC_H

#include <QMainWindow>
#include <QLabel>
#include <QItemSelection>

#include "pipeclient.h"
#include "jcon/json_rpc_websocket_client.h"
#include "dlgiperf.h"
#include "udpreceiver.h"
//#include "tpchart.h"
#include "endpointmgr.h"
#include "formendpoints.h"
#include "tpmgr.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QIperfC : public QMainWindow
{
    Q_OBJECT

public:
    QIperfC(QWidget *parent = nullptr);
    ~QIperfC() override;
public slots:
    void onNewMessage(const QString msg);
    void on_pairAdd();
    void on_pairEdit();
    void on_pairDelete();
    void on_notice(QString send_addr, QString msg);

signals:
    void updateEndpointNum(int n);

private slots:
    void init_actions();
    void initStatusbar();
    void on_pushButton_clicked();
    void on_pb_add_server_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();
//    void On_itemSelectionChanged();
    void on_updateEndpointNum(int n);
    void onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::MainWindow *ui;
    DlgIperf * dlgiperf;  // dialog of iperf config
    FormEndPoints * formEndpoits;
    PipeClient *pclient;
    jcon::JsonRpcWebSocketClient *rpc_client;
    UdpReceiver *m_receiver;
//    QChartView *m_tpchart;
//    TPChart *m_tpchart;
    EndPointMgr *m_endpointmgr;
    QLabel *m_endpoint_label;
    TPMgr *m_tpmgr;

};
#endif // QIPERFC_H
