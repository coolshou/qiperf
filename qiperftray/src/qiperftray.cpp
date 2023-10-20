#include "qiperftray.h"
#include "ui_qiperftray.h"

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
    setWindowFlags(Qt::WindowTitleHint|Qt::Dialog);
#if defined (Q_OS_LINUX)
    setFixedSize(500,300);
#endif
    ui->te_error->setVisible(false);
    m_tray = tray;

    pclient = new PipeClient(QIPERFD_NAME);
    QObject::connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
    QObject::connect(pclient, SIGNAL(sigError(QString)), this, SLOT(onError(QString)));
    pclient->SetAppHandle(qApp);

    onGetMgrIfname();
    QObject::connect(ui->pb_setMgrIfname, SIGNAL(clicked()), this, SLOT(onSetMgrIfname()));
    QObject::connect(ui->pb_getMgrIfname, SIGNAL(clicked()), this, SLOT(onGetMgrIfname()));

    statuser = new QTimer();
    QObject::connect(statuser, SIGNAL(timeout()), this, SLOT(onTimeout()));
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

void QIperfTray::statusmsg(QString msg)
{
    ui->statusBar->showMessage(msg);
}

void QIperfTray::onNewMessage(const QString msg)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        onError("");
        // handle return message
        QVariantMap result = doc.toVariant().toMap();
        QString act = result["CMD"].toString();
        if (QString::compare(act, CMD_IFNAMES, Qt::CaseInsensitive)==0){
            QVariantMap ifnameMap = result[CMD_IFNAMES].toMap();
            QStringList ifnames = ifnameMap["ifnames"].toStringList();
            ui->cb_mgr_ifnames->clear();
            ui->cb_mgr_ifnames->addItems(ifnames);
            QString ifname =  result["ifname"].toString();
            qDebug() << "current ifname:" << ifname << Qt::endl;
            int curidx = ui->cb_mgr_ifnames->currentIndex();
            int fidx =ui->cb_mgr_ifnames->findText(ifname);
            if (curidx != fidx){
                ui->cb_mgr_ifnames->setCurrentIndex(fidx);
            }
        }else if (QString::compare(act, CMD_STATUS, Qt::CaseInsensitive)==0){
            QVariantMap status = result[CMD_STATUS].toMap();
            QString works = status["iperfworkers"].toString();
            qDebug() << "CMD_STATUS:" << status << Qt::endl;
            statusmsg("iperf: " + works);

        }else {
            qDebug() << "onNewMessage:" << msg << Qt::endl;
            ui->te_msg->setText(msg.toUtf8());
        }
    }
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

void QIperfTray::onSetMgrIfname()
{    //Set Mgr_Ifname
    QString ifname = ui->cb_mgr_ifnames->currentText();
    QJsonObject jobj;
    jobj.insert("Action", CMD_SET_IFNAME);
    jobj.insert(CMD_SET_IFNAME, ifname);
    QJsonDocument doc;
    doc.setObject(jobj);
    QString strjson(doc.toJson(QJsonDocument::Compact));
    qDebug()<< "onSetMgrIfname:" << strjson << Qt::endl;
    pclient->send_MessageToServer(strjson);
}

void QIperfTray::onGetMgrIfname()
{
    pclient->send_MessageToServer(CMD_IFNAMES);
}
void QIperfTray::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    //TODO: close app check
//    event.
    savecfg();

}

void QIperfTray::onTimeout()
{
    //check daemon status
    pclient->send_MessageToServer(CMD_STATUS);
}

