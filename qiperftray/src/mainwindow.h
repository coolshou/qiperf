#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "src/mytray.h"
#include "pipeclient.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(MyTray *tray, QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void onTrayIconActivated();

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onTimeout();
    void onNewMessage(const QString msg);
    void onError(QString msg);


private:
    Ui::MainWindow *ui;
    MyTray *m_tray;
    PipeClient *pclient;
    QTimer *statuser; //timer to check daemon
};
#endif // MAINWINDOW_H
