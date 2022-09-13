#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QSysInfo>
#include <QNetworkInterface>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);

    QString arch = QSysInfo::buildCpuArchitecture();
//    onLog("arch: " + arch);
    QString ipaddr("");
    //localhost, exclude
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    const QHostAddress &localhost6 = QHostAddress(QHostAddress::LocalHostIPv6);
    foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces()) {
        QString name = netInterface.name();
        foreach (const QNetworkAddressEntry &address, netInterface.addressEntries()) {
            if ((QString::compare(address.ip().toString() , localhost.toString(), Qt::CaseInsensitive)==0)||
                (QString::compare(address.ip().toString() , localhost6.toString(), Qt::CaseInsensitive)==0)){
                //ignore
            }else{
//                onLog(name + ": " +address.ip().toString());
                ipaddr=ipaddr+ name + ":" +address.ip().toString()+"\n";
            }
        }
    }
    int idx = ipaddr.lastIndexOf("\n");
//    ui->lb_info->setText(ipaddr.left(idx));
    ui->te_info->setPlainText(ipaddr.left(idx));
//    ui->te_info->
#if defined (Q_OS_ANDROID)
    // android path
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
    QFile i2File(":/android/"+arch+"/iperf");
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
        }
    }
    //iperf3
    QFile i3File(":/android/"+arch+"/iperf3");
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
#if defined (Q_OS_ANDROID)
#else
    #if defined (Q_OS_LINUX)

    //Linux iperf binary
        //iperf2
        QFile i2File(":/linux/"+arch+"/iperf");
        QFile i3File(":/linux/"+arch+"/iperf3");

    #endif
#endif
#if defined (Q_OS_WIN32)
//windows, multi files
    QString apppath = qApp->applicationDirPath();
    m_iperfexe2 =
    m_iperfexe3
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pb_run_clicked()
{
    if (ui->pb_run->text()=="Run"){
        int ver=0;
        uint port;
        QString m_cmd;
        if (ui->rb_v2->isChecked()){
            ver=2;
            port= 5001;
#if defined (Q_OS_ANDROID)
            m_cmd = m_iperfexe2;
#else
            m_cmd = "iperf2";
#endif
        }
        if (ui->rb_v3->isChecked()){
            ver=3;
            port=5201;
#if defined (Q_OS_ANDROID)
            m_cmd = m_iperfexe3;
#else
            m_cmd = "iperf3";
#endif
        }
        QString args="-s -i 1";
        if (ver==3){
            args=args + " --forceflush";
        }

        port = ui->sb_port->value();
        iperf_th = new QThread();
//        onLog("cmd: " + m_cmd);
        iperfer = new IperfWorker(ver, m_cmd, args, port);
        connect(iperfer, SIGNAL(onStdout(QString)), this, SLOT(readStdOut(QString)));
        connect(iperfer, SIGNAL(onStderr(QString)), this, SLOT(readStdErr(QString)));
        connect(iperfer, SIGNAL(log(QString)), this, SLOT(onLog(QString)));
        connect(iperfer, SIGNAL(started()), this, SLOT(onStarted()));
        connect(iperfer, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
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
    ui->te_log->append("iperf finish: ("+ QString(exitCode)+ "): " + QString(exitStatus));
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

