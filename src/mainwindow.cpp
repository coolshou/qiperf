#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QClipboard>
#include <QScreen>
#include <QMenu>
#include <QAction>
#include <QScreen>
#include <QRect>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->rb_v21->setVisible(false);

    QMenu *menucfg = new QMenu();
    QAction *actCfg = new QAction(QIcon(":/config"), "Option");
    connect(actCfg, &QAction::triggered, this, &MainWindow::onShowCfg);

    QAction *actConsole = new QAction(QIcon(":/console"), "Console");
    connect(actConsole, &QAction::triggered, this, &MainWindow::onShowConsole);
    actConsole->setVisible(isBigScreen());

    QAction *actHelp = new QAction(QIcon(":/help"), "Help");
    connect(actHelp, &QAction::triggered, this, &MainWindow::onShowHelp);


    QList<QAction*> list;
    list.append(actCfg);
    list.append(actConsole);
    list.append(actHelp);
    menucfg->addActions(list);
    ui->pb_cfg->setMenu(menucfg);

#ifndef Q_OS_ANDROID
    move(screen()->geometry().center() - frameGeometry().center());
#endif

    cfg=new QSettings(this);
    loadcfg();

    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString arch = QSysInfo::buildCpuArchitecture();
//    onLog("arch: " + arch);
    update_ipaddrs();
//    FormOption *option = new FormOption(cfg, m_interfaces);
    option = new FormOption(cfg, m_interfaces);
    m_agent= new Agent(this, QHostAddress(m_ip), m_port, true);
    QObject::connect(option, &FormOption::ipaddressUpdated, m_agent, &Agent::updateIpaddress);
    QObject::connect(m_agent, &Agent::receivedMessage, this, &MainWindow::onReceivedMessage);

//#if defined (Q_OS_ANDROID)
#if defined (Q_OS_LINUX)
    // linux/android path
//    m_path = "/data/local/tmp";
    m_iperfexe2 = tmp +"/iperf";
    if (QFileInfo::exists(m_iperfexe2)){
        QFile::remove(m_iperfexe2);
    }
    m_iperfexe3 = tmp +"/iperf3";
    if (QFileInfo::exists(m_iperfexe3)){
        QFile::remove(m_iperfexe3);
    }
    //iperf2
    #if defined (Q_OS_ANDROID)
        QFile i2File(":/android/"+arch+"/iperf");
    #else
        QFile i2File(":/linux/"+arch+"/iperf");
    #endif
//    onLog("iperf2: " + i2File.fileName());
    if (!i2File.open(QIODevice::ReadOnly)){
        onLog("could not open " + i2File.fileName()) ;
    }else{
        if (!i2File.copy(m_iperfexe2)){
            onLog("copy iperf3 "+ i2File.fileName()+ " to "+ m_iperfexe2 + " fail");
        } else {
            // make file execuable
            QFile iperf2File(m_iperfexe2);
            iperf2File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner| QFileDevice::ExeOwner|
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup| QFileDevice::ExeGroup|
                                      QFileDevice::ReadOther | QFileDevice::WriteOther| QFileDevice::ExeOther);
    // QFileDevice::WriteOwner|QFileDevice::WriteGroup|QFileDevice::WriteOther
        }
    }
    //iperf3
    #if defined (Q_OS_ANDROID)
        QFile i3File(":/android/"+arch+"/iperf3");
    #else
        QFile i3File(":/linux/"+arch+"/iperf3");
    #endif
//    onLog("iperf2: " + i3File.fileName());
    if (!i3File.open(QIODevice::ReadOnly)){
        onLog("could not open " + i3File.fileName()) ;
    }else{
        if (!i3File.copy(m_iperfexe3)){
            onLog("copy iperf3 "+ i3File.fileName()+ " to "+ m_iperfexe3 + " fail");
        } else {
            // make file execuable
            QFile iperf3File(m_iperfexe3);
            iperf3File.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner| QFileDevice::ExeOwner|
                                      QFileDevice::ReadGroup | QFileDevice::WriteGroup| QFileDevice::ExeGroup|
                                      QFileDevice::ReadOther | QFileDevice::WriteOther| QFileDevice::ExeOther);
        }
    }
#endif
#if defined (Q_OS_WIN32)
//windows, multi files
    QString apppath = qApp->applicationDirPath();
    m_iperfexe2 = apppath + "/windows/x86/iperf.exe";
    m_iperfexe3 = apppath + "/windows/" +arch+ "/iperf3.exe";
#endif
    // agent server;
//    m_server = new MaiaXmlRpcServer(58014, this);
    //register method

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isBigScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    qreal dpi = screen->physicalDotsPerInch();
    int height = screenGeometry.height();
    int width = screenGeometry.width();
//    qDebug() << "height:" << QString::number(height)
//             << " width:" << QString::number(width)
//             << " dpi:" << QString::number(dpi)
//             << Qt::endl;
    if ((qSqrt(qPow(height, 2) + qPow(width, 2)) / dpi) > 10){
        qDebug() << "screen over 10' size" << Qt::endl;
        return true;
    }else{
        return false;
    }
}

void MainWindow::onReceivedMessage(QString message)
{
    qDebug() << "message:" << message << Qt::endl;
}


void MainWindow::on_pb_run_clicked()
{
    if (ui->pb_run->text()=="Run"){
        if (ui->cb_client->isChecked()){
            //check client mode host value
            if (ui->le_host->text().isEmpty()){
                onLog("Please specift Target Host to connect!!");
                ui->le_host->setFocus();
                return;
            } else {
                //check host format
                QHostAddress address(ui->le_host->text());
                if ((QAbstractSocket::IPv4Protocol == address.protocol()) ||
                    (QAbstractSocket::IPv6Protocol == address.protocol())){
                    if (address.isLinkLocal()) {
                        onLog("ipv6 link local must include interface name!!");
                    }
                }else{
                    onLog("Wrong Target Host Address!!");
                    ui->le_host->setFocus();
                    return;
                }
            }
        }
        int ver=0;
        uint port;
        QString m_cmd;
        if (ui->rb_v2->isChecked()){
            ver=2;
            port= 5001;
//#if defined (Q_OS_LINUX)
            m_cmd = m_iperfexe2;
//#else
//            m_cmd = "iperf2";
//#endif
        }
        //TODO iperf2.1
        if (ui->rb_v3->isChecked()){
            ver=3;
            port=5201;
//#if defined (Q_OS_LINUX)
            m_cmd = m_iperfexe3;
//#else
//            m_cmd = "iperf3";
//#endif
        }
        QString args;
        if (ui->cb_client->isChecked()){
            args =args + cfg->value("args").toString();
            args =args + " -c " + ui->le_host->text();
            if (ver==3){
                args =args + " -i 1 ";
            }
        }else{
            args =args +"-s";
        }
//        if (ver==3){
//            args= args+" --forceflush ";
//        }

        port = ui->sb_port->value();
        iperf_th = new QThread();
//        onLog("cmd: " + m_cmd);
        iperfer = new IperfWorker(ver, m_cmd, args, port);
        connect(iperfer, SIGNAL(onStdout(QString)), this, SLOT(readStdOut(QString)));
        connect(iperfer, SIGNAL(onStderr(QString)), this, SLOT(readStdErr(QString)));
        connect(iperfer, SIGNAL(log(QString)), this, SLOT(onLog(QString)));
        connect(iperfer, SIGNAL(started()), this, SLOT(onStarted()));
        connect(iperfer, SIGNAL(finished(int, int)), this, SLOT(onFinished(int, int)));
        iperfer->moveToThread(iperf_th);
        connect(iperf_th, SIGNAL(started()), iperfer, SLOT(work()));
        iperf_th->start();
    } else{
//        onLog("Stop iperf");
        iperfer->setStop();
        ui->pb_run->setText("Run");
    }
}

void MainWindow::readStdOut(QString text)
{
    QStringList lines= text.split("\n");
    for ( const auto& line : lines  ){
        if (line.length()>0){
            ui->te_log->append(line);
        }
    }
}

void MainWindow::readStdErr(QString text)
{
    ui->te_log->append(text);
}

void MainWindow::onStarted()
{
    ui->pb_run->setText("Stop");

}

void MainWindow::onFinished(int exitCode, int exitStatus)
{
    ui->pb_run->setText("Run");
    ui->te_log->append("iperf finish: ("+ QString::number(exitCode)+ "): " + QString::number(exitStatus));
}

void MainWindow::onLog(QString text)
{
    ui->te_log->append(text);
}


void MainWindow::on_pb_clear_clicked()
{
    //clear log
    ui->te_log->clear();
}


void MainWindow::on_pb_quit_clicked()
{
    qApp->exit(0);
}


void MainWindow::on_pb_copy_clicked()
{
    //copy all log to clipboard
    qApp->clipboard()->setText(ui->te_log->toPlainText());
}


void MainWindow::on_rb_v2_clicked()
{
    ui->sb_port->setValue(5001);
}


void MainWindow::on_rb_v3_clicked()
{
    ui->sb_port->setValue(5201);
}


void MainWindow::on_rb_v21_clicked()
{
    ui->sb_port->setValue(5001);
}


void MainWindow::on_pb_cfg_clicked()
{   //When menu assign to QButton, this will not trigger!

//    QRect rect = this->geometry();
//    qDebug() << "rect:" <<rect << Qt::endl;
//    FormOption *option;
//    option =new FormOption(cfg);
//    option->setGeometry(rect);
////    option->deleteLater();
//    option->show();
}

void MainWindow::onShowCfg()
{
    QRect rect = this->geometry();
    option->setGeometry(rect);
    option->show();
}

void MainWindow::onShowConsole()
{
#if !defined (Q_OS_ANDROID)
    FormConsole *console = new FormConsole(cfg);
    console->show();
#endif
}

void MainWindow::onShowHelp()
{
    //TODO: show help
}


void MainWindow::on_cb_client_stateChanged(int arg1)
{
    bool state;
    if (arg1==Qt::Unchecked) {
        state = false;
    }else{
        state = true;
    }
    ui->w_client->setEnabled(state);
    ui->w_info->setVisible(!state);
}

void MainWindow::update_ipaddrs()
{
    QString ipaddr("");
    //localhost, exclude
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    const QHostAddress &localhost6 = QHostAddress(QHostAddress::LocalHostIPv6);
    foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces()) {
        QString name = netInterface.name();
//        qDebug() << "type: " << netInterface.type() <<" : " << netInterface.humanReadableName().toUpper() << Qt::endl;
        //filter some interface
        if (name.toUpper().contains("LXCB")||name.toUpper().contains("VIRBR")
            ||name.toUpper().contains("DOCKER")||name.toUpper().contains("VBOXNET")){
            continue;
        }
        foreach (const QNetworkAddressEntry &address, netInterface.addressEntries()) {
            if ((QString::compare(address.ip().toString() , localhost.toString(), Qt::CaseInsensitive)==0)||
                (QString::compare(address.ip().toString() , localhost6.toString(), Qt::CaseInsensitive)==0)){
                //ignore
            }else{
                //onLog(name + ": " +address.ip().toString());
                ipaddr=ipaddr+ name + ":" +address.ip().toString()+"\n";
                if (netInterface.type() == QNetworkInterface::Ethernet){
                    if (address.ip().protocol() == QAbstractSocket::IPv4Protocol){
                        if (m_interfaces.indexOf(name+": "+address.ip().toString())<0){
                            //not duplicate
                            m_interfaces.append(name+": "+address.ip().toString());
                        }
                    }
                }
            }
        }
    }
    int idx = ipaddr.lastIndexOf("\n");
    ui->te_info->setPlainText(ipaddr.left(idx));
}

void MainWindow::loadcfg()
{
    cfg->beginGroup("main");
    QString ifname = cfg->value("managerifname", QHostAddress(QHostAddress::LocalHost).toIPv4Address()).toString();
    if (!ifname.isEmpty()){
        if (ifname.contains(":")){
            QStringList qs = ifname.split(":");
            qDebug() << " if: " << qs[0] << "ip: " << qs[1] << Qt::endl;
            m_ip = qs[1];
        }else{
            m_ip = ifname;
        }
    }
    m_port = cfg->value("managerport", 45454).toInt();
    cfg->endGroup();
}

