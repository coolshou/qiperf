#ifndef QIPERFTRAY_H
#define QIPERFTRAY_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>

#include "src/mytray.h"
#include "pipeclient.h"
#include "../src/dlgshowlog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QIperfTray : public QMainWindow
{
    Q_OBJECT

public:
    QIperfTray(MyTray *tray, QWidget *parent = nullptr);
    ~QIperfTray() override;
    void loadcfg();
    void savecfg();
    void setLogFile(QString filename);
    void statusmsg(QString msg);
    void statusQiperfd();
    void getQiperfdLogFile();
    void showNotice(QString title, QString msg);
    void showError(QString title, QString msg);
signals:
    void updateIperfcount(int count);
    void updateQIperfd(QString msg);

public slots:
    void onTrayIconActivated();
    void onSetMgrIfname();
    void onGetMgrIfname();
    void startQiperfd();
    void stopQiperfd();
    void restartQiperfd();
    void onShowQiperfdLog();
    void onShowLog();
    void onAbout();
    void onNotice();
    void onIperfVersion();

protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onTimeout();
    void onNewMessage(const QString msg);
    void onError(QString msg);
    void initActions();
    void onIfnameChange(int index);
    void initStatusbar();
    void onUpdateIperfcount(int count);
    void onUpdateQIperfd(QString msg);

private:
    QSettings *cfg;
    QRect m_geometry;
    Ui::MainWindow *ui;
    MyTray *m_tray;
    PipeClient *pclient;
    QTimer *statuser; //timer to check daemon
    DlgShowLog *m_dlgshowqiperfdlog;
    DlgShowLog *m_dlgshowlog;
    QString m_qiperfdlog;
    QLabel *m_iperfcount;
    QLabel *m_qiperfd;
    QLabel *m_status;
};
#endif // QIPERFTRAY_H
