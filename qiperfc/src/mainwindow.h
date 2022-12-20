#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "pipeclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
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
};
#endif // MAINWINDOW_H
