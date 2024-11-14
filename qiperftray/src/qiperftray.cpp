#include "qiperftray.h"
#include "qiperftray.h"
#include "ui_qiperftray.h"

#include "comm.h"
#include "version.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

#include <QDebug>

QIperfTray::QIperfTray(MyTray *tray, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSettings cfg= QSettings(QSettings::IniFormat, QSettings::UserScope,
                              QIPERF_ORG, QIPERFTRAY_NAME);
    //SystemScope: /etc/xdg/xdg-lxqt/alphanetworks/qiperftray.conf
    //  sudo =>      /etc/xdg/alphanetworks/qiperftray.conf
    //UserScope: /home/jimmy/.config/alphanetworks/qiperftray.conf
    //  sudo =>       /root/.config/alphanetworks/qiperftray.conf
    //Windows:
    //  /c:/user/<xxx>/appdata/local/temp/qiperf/
    ui->setupUi(this);
    loadcfg();

    //DENUG use
    ui->menuTest->menuAction()->setVisible(false);

    // ui->menuTest->menuAction()hide();
    //qiperfd log path, this will get wrong path if qiperfd is run under administrator => c:\windows\temp
//    QString qiperfd =  QStandardPaths::writableLocation(QStandardPaths::TempLocation);
//    m_qiperfdlog = qiperfd + QDir::separator() + QIPERF_NAME + QDir::separator() + QIPERFD_NAME + ".log";
    m_dlgshowlog = new DlgShowLog();

//    qDebug() << "m_qiperfdlog: " << m_qiperfdlog;

    //TODO: get install path!! or exec path?

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

    QObject::connect(ui->pb_setMgrIfname, SIGNAL(clicked()), this, SLOT(onSetMgrIfname()));
    QObject::connect(ui->pb_getMgrIfname, SIGNAL(clicked()), this, SLOT(onGetMgrIfname()));
    onGetMgrIfname();
    getQiperfdLogFile();
    initActions();

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

void QIperfTray::startQiperfd()
{
    //TODO: start Qiperfd service
    // nssm.exe start "qiperfd"
    //sudo systemctl start qiperfd.service
}

void QIperfTray::stopQiperfd()
{
    //TODO: stop Qiperfd service
    // nssm.exe stop "qiperfd"
    //sudo systemctl stop qiperfd.service
}

void QIperfTray::restartQiperfd()
{
    //restart Qiperfd service
}

void QIperfTray::statusQiperfd()
{
    //get status of qiperfd, 0: stop , 1: running
}

void QIperfTray::getQiperfdLogFile()
{
    pclient->send_MessageToServer(CMD_GET_LOGFILENAME);
}

void QIperfTray::onNewMessage(const QString msg)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
//        onError("");
        // handle return message
        QVariantMap result = doc.toVariant().toMap();
        QString act = result["CMD"].toString();
        if (QString::compare(act, CMD_IFNAMES, Qt::CaseInsensitive)==0){
            QVariantMap ifnameMap = result[CMD_IFNAMES].toMap();
            QStringList ifnames = ifnameMap["ifnames"].toStringList();
            ui->cb_mgr_ifnames->clear();
            ui->cb_mgr_ifnames->addItems(ifnames);
            QString ifname =  result["ifname"].toString();
//            qDebug() << "current ifname:" << ifname << Qt::endl;
            int curidx = ui->cb_mgr_ifnames->currentIndex();
            int fidx =ui->cb_mgr_ifnames->findText(ifname);
            if (curidx != fidx){
                ui->cb_mgr_ifnames->setCurrentIndex(fidx);
            }
        }else if (QString::compare(act, CMD_STATUS, Qt::CaseInsensitive)==0){
            QVariantMap status = result[CMD_STATUS].toMap();
            QString works = status["iperfworkers"].toString();
//            qDebug() << "CMD_STATUS:" << status << Qt::endl;
            statusmsg("iperf: " + works);

        }else {
//            qDebug() << "onNewMessage:" << msg << Qt::endl;
            ui->te_msg->setText(msg.toUtf8());
        }
    }else {
        if (msg.contains("：")) {
            qInfo() << "onNewMessage:" << msg;
            QStringList ds = msg.split("：");
            if (ds.length()==2){
                if (ds.value(0)==QIPERFDLOG){
                    m_dlgshowlog->appendNewLine(ds.value(1));
                }else if (ds.value(0)==CMD_GET_LOGFILENAME){
                    m_qiperfdlog = ds.value(1);
                    qDebug() << "m_qiperfdlog: " << m_qiperfdlog;
                    m_dlgshowlog->setLogFile(m_qiperfdlog);
                }
            }else {
                qDebug() << "onNewMessage: Unknown format :" << msg;
            }
        }else {
            qDebug() << "onNewMessage: ERROR format of msg: ' " << msg << " '";
        }
    }
}

void QIperfTray::onError(QString msg)
{
    if (!msg.isEmpty()){
//        qDebug() << "onError: " << msg;
        ui->te_error->setText(msg);
        ui->te_error->setVisible(true);
    } else {
        ui->te_error->setVisible(false);
    }
}

void QIperfTray::initActions()
{
    // connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    // connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(onRestart()));
    connect(ui->actionShowLog, SIGNAL(triggered()), this, SLOT(onShowLog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));

    connect(ui->actionNotice, SIGNAL(triggered()), this, SLOT(onNotice()));

}

void QIperfTray::onTrayIconActivated()
{
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
//    qDebug()<< "onSetMgrIfname:" << strjson << Qt::endl;
    pclient->send_MessageToServer(strjson);
}

void QIperfTray::onGetMgrIfname()
{
    pclient->send_MessageToServer(CMD_IFNAMES);
}

void QIperfTray::onStart()
{
    //tell qiperfd start
    pclient->send_MessageToServer(CMD_QIPERFD_START);
}

void QIperfTray::onStop()
{
    //tell qiperfd stop
    pclient->send_MessageToServer(CMD_QIPERFD_STOP);
}

void QIperfTray::onRestart()
{
    //tell qiperfd stop
    pclient->send_MessageToServer(CMD_QIPERFD_RESTART);
}

void QIperfTray::onShowLog()
{
    // show a dialog to show qiperfd log file continious
    m_dlgshowlog->show();
}

void QIperfTray::onAbout()
{
    QMessageBox::about(this, "About", QString(QIPERFTRAY_NAME)+
                       " v"+QString(VERSION)+"\n"
                       "Auther: Jimmy Yeh\n"
                       "URL: https://github.com/coolshou/qiperf");
}

void QIperfTray::onNotice()
{
    if (m_tray->supportsMessages()){
        m_tray->showMessage("test", "notice of balloon messages");
    }else {
        qDebug() << "System Not support balloon messages!!";
    }
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

