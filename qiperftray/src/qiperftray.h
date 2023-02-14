#ifndef QIPERFTRAY_H
#define QIPERFTRAY_H

#include <QMainWindow>
#include <QTimer>
#include <QSettings>
#include <QRect>

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

public slots:
    void onTrayIconActivated();

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
