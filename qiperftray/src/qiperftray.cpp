#include "qiperftray.h"
#include "qiperftray.h"
#include "ui_qiperftray.h"

#include "comm.h"
#include "version.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QComboBox>
#include <QJsonArray>
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
    QObject::connect(ui->cb_mgr_ifnames, SIGNAL(currentIndexChanged(int)), this,SLOT(onIfnameChange(int)));

    // ui->menuTest->menuAction()hide();
    //qiperfd log path, this will get wrong path if qiperfd is run under administrator => c:\windows\temp
//    QString qiperfd =  QStandardPaths::writableLocation(QStandardPaths::TempLocation);
//    m_qiperfdlog = qiperfd + QDir::separator() + QIPERF_NAME + QDir::separator() + QIPERFD_NAME + ".log";
    m_dlgshowqiperfdlog = new DlgShowLog(this);
    m_dlgshowlog = new DlgShowLog(this);
//    qDebug() << "m_qiperfdlog: " << m_qiperfdlog;

    //TODO: get install path!! or exec path?

    // setWindowFlags(Qt::WindowTitleHint|Qt::Dialog);
#if defined (Q_OS_LINUX)
    setFixedSize(800,300);//TODO: should we fix the window size?
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
    if (m_dlgshowqiperfdlog){
        m_dlgshowqiperfdlog->close();
    }
    if (m_dlgshowlog){
        m_dlgshowlog->close();
    }
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

void QIperfTray::setLogFile(QString filename)
{
    m_dlgshowlog->setLogFile(filename);
}

void QIperfTray::statusmsg(QString msg)
{
    ui->statusBar->showMessage(msg);
}

void QIperfTray::statusQiperfd()
{
    //get status of qiperfd, 0: stop , 1: running
}

void QIperfTray::getQiperfdLogFile()
{
    pclient->send_MessageToServer(CMD_GET_LOGFILENAME);
}

void QIperfTray::showNotice(QString title, QString msg)
{
    if (m_tray->supportsMessages()){
        m_tray->showMessage(title, msg);
    }else {
        qDebug() << "System Not support balloon messages!!";
    }
}

void QIperfTray::showError(QString title, QString msg)
{
    if (m_tray->supportsMessages()){
        m_tray->showMessage(title, msg, QSystemTrayIcon::Warning);
    }else {
        qDebug() << "System Not support balloon messages!!";
    }
}

void QIperfTray::onNewMessage(const QString msg)
{   // handle qiperfd message
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
            // qDebug() << "onNewMessage:CMD_IFNAMES: " << result["netobj"];

            QJsonDocument net = QJsonDocument::fromJson(result["netobj"].toString().toUtf8());
            QJsonObject netobj = net.object();//.toJsonObject();
            // qDebug() << "onNewMessage:netobj: " << netobj;

            QString ifname;
            ui->cb_mgr_ifnames->clear();
            ui->cb_mgr_ifnames->addItems(ifnames);
            //set first ip address
            QJsonObject data;
            QJsonArray addrsarray;
            QJsonArray addrarr;
            for (int i =0; i< ui->cb_mgr_ifnames->count(); i++ ){
                ifname = ui->cb_mgr_ifnames->itemText(i);
                data = netobj.value(ifname).toObject();
                addrsarray = data.value("address").toArray();
                // qDebug() << "addrsarray:" << addrsarray;
                if (addrsarray.count()>0){
                    if (addrsarray.at(0).isArray()){
                        addrarr = addrsarray.at(0).toArray();// TODO: multi address handle
                        // qDebug() << "addrarr:" << addrarr;
                        if (addrarr.count()>0){
                            ui->cb_mgr_ifnames->setItemData(i, addrarr[0].toString());
                        }
                    }
                }

            }

            //change combobox to result["ifname"]
            ifname =  result["ifname"].toString();
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
                    m_dlgshowqiperfdlog->appendNewLine(ds.value(1));
                }else if (ds.value(0)==CMD_GET_LOGFILENAME){
                    m_qiperfdlog = ds.value(1);
                    qDebug() << "m_qiperfdlog: " << m_qiperfdlog;
                    m_dlgshowqiperfdlog->setLogFile(m_qiperfdlog);
                }
            }else {
                qDebug() << "onNewMessage: Unknown format :" << msg;
            }
        }else {
            if (msg == INFO_QIPERFD_STOPED){
                showNotice("Info", "qiperfd stoped!!");
            }
            if (msg == INFO_QIPERFD_STARTED){
                showNotice("Info", "qiperfd started!!");
            }
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
        showError("ERROR", msg);
    } else {
        ui->te_error->setVisible(false);
    }
}

void QIperfTray::initActions()
{
    // connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onStart()));
    // connect(ui->actionStop, SIGNAL(triggered()), this, SLOT(onStop()));
    connect(ui->actionRestart, SIGNAL(triggered()), this, SLOT(restartQiperfd()));
    connect(ui->actionShowQIperfdLog, SIGNAL(triggered()), this, SLOT(onShowQiperfdLog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));
    connect(ui->actionShowLog, SIGNAL(triggered()), this, SLOT(onShowLog()));

    connect(ui->actionNotice, SIGNAL(triggered()), this, SLOT(onNotice()));

}

void QIperfTray::onIfnameChange(int index)
{
    QString address = ui->cb_mgr_ifnames->itemData(index).toString();
    ui->lb_address->setText(address);
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
    int idx = ui->cb_mgr_ifnames->currentIndex();
    QString address = ui->cb_mgr_ifnames->itemData(idx).toString();
    if (address.isEmpty()){
        showError("Error", "Please setup manager interface \""+ifname+"\"with IP address");
        return;
    }
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

void QIperfTray::startQiperfd()
{
    //tell qiperfd start
    pclient->send_MessageToServer(CMD_QIPERFD_START);
}

void QIperfTray::stopQiperfd()
{
    //tell qiperfd stop
    pclient->send_MessageToServer(CMD_QIPERFD_STOP);
}

void QIperfTray::restartQiperfd()
{
    //tell qiperfd stop
    pclient->send_MessageToServer(CMD_QIPERFD_RESTART);
}

void QIperfTray::onShowQiperfdLog()
{
    // show a dialog to show qiperfd log file continious
    m_dlgshowqiperfdlog->show();
}

void QIperfTray::onShowLog()
{
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

