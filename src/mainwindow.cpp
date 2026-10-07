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
    ui->rb_v2->setVisible(false);
    // mThreadId = nullptr;
    // 1. Create the wrapper object
    wrapper= new Iperf3Wrapper(parent);
    // 2. Connect signals to handle results and errors
    connect(wrapper, &Iperf3Wrapper::testFinished, this, &MainWindow::onTestFinished);
    connect(wrapper, &Iperf3Wrapper::testError, this, &MainWindow::onTestError);
    connect(wrapper, &Iperf3Wrapper::testLog, this, &MainWindow::onLog);
    connect(wrapper, &Iperf3Wrapper::testStarted, this, &MainWindow::onTestStarted);
    connect(wrapper, &Iperf3Wrapper::started,  this, &MainWindow::onStarted);
    connect(wrapper, &Iperf3Wrapper::iperfIntervalReport, this, &MainWindow::onIperfIntervalReport);

    QStringList ipaddres;
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    foreach (const QNetworkInterface &netInterface, QNetworkInterface::allInterfaces()) {
        QString name = netInterface.name();
        foreach (const QNetworkAddressEntry &address, netInterface.addressEntries()) {
            if (QString::compare(address.ip().toString() , localhost.toString(), Qt::CaseInsensitive)!=0){
                ipaddres.append(name + ":"+address.ip().toString());
            }
        }
    }
    ui->cb_info->addItems(ipaddres);

    connect(ui->pbStart, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(ui->pbStop, &QPushButton::clicked, this, &MainWindow::onStop);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readStdOut(QString text)
{
    QStringList lines= text.split("\n");
    for ( const auto& line : lines  ){
        if (line.length()>0){
            ui->te_log->appendPlainText(line);
        }
    }
}

void MainWindow::readStdErr(QString text)
{
    ui->te_log->appendPlainText(text);
}

void MainWindow::onFinished(int exitCode, int exitStatus)
{
    // ui->pb_run->setText("Run");
    ui->te_log->appendPlainText("iperf finish: ("+ QString::number(exitCode)+ "): " + QString::number(exitStatus));
}

void MainWindow::onLog(const QString &msg)
{
    ui->te_log->appendPlainText(msg);
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
void MainWindow::onTestFinished(const QString& jsonResult)
{
    onLog(jsonResult);
    ui->pbStart->setEnabled(true);
    ui->pbStop->setEnabled(false);
    // mThreadId = nullptr;
}

void MainWindow::onTestError(const QString &errorMessage)
{
    onLog("--- TEST FAILED ---");
    qCritical() << "Error:" << errorMessage;
    onLog(errorMessage);
}

void MainWindow::onTestStarted(quintptr threadId)
{
    onLog("iperf3 started threadid:" + QString::number(threadId));
    mThreadId = threadId;
    onStarted();
}

void MainWindow::onStarted()
{
    ui->pbStart->setEnabled(false);
    ui->pbStop->setEnabled(true);
}

void MainWindow::onIperfIntervalReport(const QList<StreamMetrics> &streamList)
{
    for (const StreamMetrics &metrics : streamList) {
        int id = metrics.streamId;
        QString unitf = metrics.unit_format;
        // double dUnit = 0.0;
        // dUnit = getUnitFormat(metrics.unit_format, unitf);
        // double speedMbps = metrics.bandwidth / dUnit; // Convert bits to Mbps
        double speedMbps = metrics.bandwidth ;
        int iDecimal=0;
        if (speedMbps>=10000){
            iDecimal =0;
        }else if (speedMbps>=1000){
            iDecimal =1;
        }else {
            iDecimal =2;
        }
        // long long bytes = metrics.bytesTransferred;

        // Differentiate between Upload and Download streams
        QString direction = metrics.isSender ? "TX" : "RX";
        QString omit = metrics.omitted ? "omit": "";
        // Example: Print to debug console
        QString msg = QString("Stream [%1]%2-%3 (%4): %5 %6 (%7/%8 (%9%)) %10")
                          .arg(id).arg(metrics.start_time, 0, 'f', 1)
                          .arg(metrics.start_time+metrics.end_time, 0, 'f', 1)
                          .arg(direction).arg(speedMbps, 0, 'f', iDecimal).arg(unitf)
                          .arg(metrics.interval_cnt_error).arg(metrics.interval_cnt_error)
                          .arg(metrics.lost_percent, 0, 'f', 0)
                          .arg(omit);
        onLog(msg);
    }
}
void MainWindow::onStart(bool checked)
{
    Q_UNUSED(checked)
    if (wrapper){
        // ui->cb_ServerMode
        serverHost = ui->le_target->text();
        serverPort = ui->sb_port->value();
        // duration
        // streams
        isClient = !ui->cb_ServerMode->isChecked();

        if (!wrapper->startTest(serverHost, serverPort, duration,
                                streams, isClient)) {
            onLog("Failed to start test.");
        }
    }
}

void MainWindow::onStop(bool checked)
{
    Q_UNUSED(checked)
    if (wrapper){
        wrapper->stopTest();
    }

}
