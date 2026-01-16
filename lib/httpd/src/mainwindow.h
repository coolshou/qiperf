#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

#include "myhttpserver.h"
#include "myhttpserverform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void closeAll(); // send signal to close all dialog
protected:
    void changeEvent(QEvent *e);

private:
    Ui::MainWindow *ui;
    QSettings *m_settings;
    ServerConfig *config;
    MyHttpServer *httpsrv;
    MyHttpServerForm *httpfrm;
};

#endif // MAINWINDOW_H
