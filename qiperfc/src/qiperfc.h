#ifndef QIPERFC_H
#define QIPERFC_H

#include <QMainWindow>
#include "pipeclient.h"
#include "jcon/json_rpc_websocket_client.h"
#include "dlgiperf.h"
#include "udpreceiver.h"
#include "tpchart.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QIperfC : public QMainWindow
{
    Q_OBJECT

public:
    QIperfC(QWidget *parent = nullptr);
    ~QIperfC();
public slots:
    void onNewMessage(const QString msg);
    void on_pairAdd();
    void on_pairEdit();
    void on_pairDelete();
    void on_notice(QString msg);
private slots:
    void init_actions();
    void on_pushButton_clicked();
    void on_pb_add_server_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();

private:
    Ui::MainWindow *ui;
    DlgIperf * dlgiperf;
    PipeClient *pclient;
    jcon::JsonRpcWebSocketClient *rpc_client;
    UdpReceiver *m_receiver;
//    QChartView *m_tpchart;
    TPChart *m_tpchart;
};
#endif // QIPERFC_H
