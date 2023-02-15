#ifndef QIPERFC_H
#define QIPERFC_H

#include <QMainWindow>
#include "pipeclient.h"
#include "jcon/json_rpc_websocket_client.h"

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
private slots:
    void on_pushButton_clicked();
    void on_pb_add_server_clicked();
    void on_pb_start_clicked();
    void on_pb_stop_clicked();

private:
    Ui::MainWindow *ui;
    PipeClient *pclient;
    jcon::JsonRpcWebSocketClient *rpc_client;
};
#endif // QIPERFC_H
