#include "qiperftray.h"
#include "ui_mainwindow.h"

#include "comm.h"
#include <QDebug>

QIperfTray::QIperfTray(MyTray *tray, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSettings cfg= QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              QIPERF_ORG, QIPERFTRAY_NAME);
    ui->setupUi(this);
    loadcfg();
//    Qt::WindowFlags flags = 0;
//    flags |= Qt::FramelessWindowHint;
//    setWindowFlags(Qt::FramelessWindowHint);
//    setWindowFlags(Qt::SplashScreen);
    setWindowFlags(Qt::WindowTitleHint|Qt::Dialog);
#if defined (Q_OS_LINUX)
    setFixedSize(300,300);
#endif
    ui->te_error->setVisible(false);
    m_tray = tray;

    pclient = new PipeClient(QIPERFD_NAME);
    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
    connect(pclient, SIGNAL(sigError(QString)), this, SLOT(onError(QString)));
    pclient->SetAppHandle(qApp);

    statuser = new QTimer();
    connect (statuser, SIGNAL(timeout()), this, SLOT(onTimeout()));
    statuser->start(5000);

}

QIperfTray::~QIperfTray()
{
    delete ui;
}

void QIperfTray::loadcfg()
{
    cfg.beginGroup("main");
    m_geometry = cfg.value("geometry", QRect(0,0,200,200)).toRect();
    cfg.endGroup();
    setGeometry(m_geometry);
}

void QIperfTray::savecfg()
{
    cfg.beginGroup("main");
    cfg.setValue("geometry", this->geometry());
    cfg.endGroup();
    cfg.sync();

}
void QIperfTray::onNewMessage(const QString msg)
{
//    ui->textEdit->append(msg);
    qDebug() << "onNewMessage:" << msg << Qt::endl;
    ui->te_msg->setText(msg);
}

void QIperfTray::onError(QString msg)
{
    if (!msg.isEmpty()){
        ui->te_error->setText(msg);
        ui->te_error->setVisible(true);
    } else {
        ui->te_error->setVisible(false);
    }
}

void QIperfTray::onTrayIconActivated()
{   qDebug() << "onTrayIconActivated" << Qt::endl;

    if (this->isVisible()){
        this->hide();
    }else{
        this->show();
    }
}
void QIperfTray::closeEvent(QCloseEvent *event)
{
    //TODO: close app check
//    event.
    savecfg();

}

void QIperfTray::onTimeout()
{
    //check daemon status
    pclient->send_MessageToServer(CMD_STATUS);
}

