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
    ui->groupBox->setVisible(false);
    connect(ui->cb_ServerMode, &QCheckBox::checkStateChanged, this, &MainWindow::onServerModeCheckStateChanged);
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
        QNetworkInterface::InterfaceFlags flags = netInterface.flags();
        // 1. 過濾未啟用 (IsUp) 或迴路介面 (IsLoopBack)
        if (!flags.testFlag(QNetworkInterface::IsUp) || flags.testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        // 2. 判斷網卡類型：僅留 Ethernet 與 Wi-Fi
        QNetworkInterface::InterfaceType type = netInterface.type();
        bool isEthernet = (type == QNetworkInterface::Ethernet);
        bool isWifi = (type == QNetworkInterface::Wifi || type == QNetworkInterface::Ieee80211);

        if (!isEthernet && !isWifi) {
            continue;
        }
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
    connect(ui->pb_clear, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->pb_quit, &QPushButton::clicked, this, &MainWindow::onQuitClicked);
    connect(ui->pb_copy, &QPushButton::clicked, this, &MainWindow::onCopyClicked);

    connect(this, &MainWindow::stop, wrapper, &Iperf3Wrapper::stopTest);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onStart(bool checked)
{
    Q_UNUSED(checked)
    if (wrapper){
        // ui->cb_ServerMode
        serverHost = ui->le_target->text();
        serverPort = ui->sb_port->value();
        if (ui->rbTCP->isChecked()){
            sProtocal = "TCP";
        }else {
            sProtocal = "UDP";
        }
        duration = ui->sb_duration->value();
        streams = ui->sb_parallel->value();
        isClient = !ui->cb_ServerMode->isChecked();

        if (!wrapper->startTest(serverHost, serverPort, sProtocal,
                                duration, streams, isClient)) {
            onLog("Failed to start test.");
        }
    }
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


void MainWindow::onClearClicked()
{
    //clear log
    ui->te_log->clear();
}


void MainWindow::onQuitClicked()
{
    qApp->exit(0);
}


void MainWindow::onCopyClicked()
{
    //copy all log to clipboard
    qApp->clipboard()->setText(ui->te_log->toPlainText());
}

void MainWindow::onServerModeCheckStateChanged(Qt::CheckState state)
{
    if (state == Qt::Checked){
        ui->wClient->setEnabled(false);
    }else{
        ui->wClient->setEnabled(true);
    }
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
        QString omit = metrics.omitted ? "(omitted)": "";
        // Example: Print to debug console
        QString msg = QString("Stream [%1]%2-%3 (%4): %5 %6")
                          .arg(id).arg(metrics.start_time, 0, 'f', 1)
                          .arg(metrics.start_time+metrics.end_time, 0, 'f', 1)
                          .arg(direction)
                          .arg(speedMbps, 0, 'f', iDecimal)
                          .arg(unitf);
        if (sProtocal.compare("UDP", Qt::CaseInsensitive)==0){
            msg = QString("%1 (%2/%3 (%4%))")
                      .arg(msg)
                      .arg(metrics.interval_cnt_error).arg(metrics.interval_cnt_error)
                      .arg(metrics.lost_percent, 0, 'f', 0);
        }
        if (!omit.isEmpty()){
             msg = QString("%1 %2").arg(msg).arg(omit);
        }

        onLog(msg);
    }
}


void MainWindow::onStop(bool checked)
{
    Q_UNUSED(checked)
    emit stop();
    // if (wrapper){
    //     wrapper->stopTest();
    // }

}
