#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "comm.h"
#include <QDebug>

MainWindow::MainWindow(MyTray *tray, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onNewMessage(const QString msg)
{
//    ui->textEdit->append(msg);
    qDebug() << "onNewMessage:" << msg << Qt::endl;
    ui->te_msg->setText(msg);
}

void MainWindow::onError(QString msg)
{
    if (!msg.isEmpty()){
        ui->te_error->setText(msg);
        ui->te_error->setVisible(true);
    } else {
        ui->te_error->setVisible(false);
    }
}

void MainWindow::onTrayIconActivated()
{   qDebug() << "onTrayIconActivated" << Qt::endl;

    if (this->isVisible()){
        this->hide();
    }else{
        this->show();
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    //TODO: close app check

}

void MainWindow::onTimeout()
{
    //check daemon status
    pclient->send_MessageToServer(CMD_STATUS);
}

