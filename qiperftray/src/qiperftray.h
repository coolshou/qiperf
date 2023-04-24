#ifndef QIPERFTRAY_H
#define QIPERFTRAY_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QRect>
#include <QJsonDocument>
#include <QJsonObject>

#include "src/mytray.h"
#include "pipeclient.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QIperfTray : public QMainWindow
{
    Q_OBJECT

public:
    QIperfTray(MyTray *tray, QWidget *parent = nullptr);
    ~QIperfTray();
    void loadcfg();
    void savecfg();
    void statusmsg(QString msg);

public slots:
    void onTrayIconActivated();
    void onSetMgrIfname();
    void onGetMgrIfname();

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void onTimeout();
    void onNewMessage(const QString msg);
    void onError(QString msg);


private:
    QSettings cfg;
    QRect m_geometry;
    Ui::MainWindow *ui;
    MyTray *m_tray;
    PipeClient *pclient;
    QTimer *statuser; //timer to check daemon
};
#endif // QIPERFTRAY_H
