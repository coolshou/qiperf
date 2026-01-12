#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardPaths>
#include <QDir>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString settingfilepath =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir d{settingfilepath};
    if (!d.exists()){
        if(!d.mkpath(settingfilepath)){
            qDebug() << "ERROR: mkdir " + settingfilepath + " Fail";
        }
    }
    QString settingfilename = settingfilepath + QDir::separator() + "myhttpd.ini";
    m_settings = new QSettings(settingfilename, QSettings::IniFormat);
    config = new ServerConfig();
    httpsrv = new MyHttpServer(config);
    httpfrm = new MyHttpServerForm(m_settings);
    // Add a layout to the central widget if not already set in Designer
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(httpfrm); // Add an existing widget to the layout
    ui->centralwidget->setLayout(layout);

    connect(httpfrm, &MyHttpServerForm::sigRootPathChange, config, &ServerConfig::setRootDir);
    connect(httpfrm, &MyHttpServerForm::sigStart, httpsrv, &MyHttpServer::start);
    connect(httpfrm, &MyHttpServerForm::sigStop, httpsrv, &MyHttpServer::stop);
    connect(httpsrv, &MyHttpServer::started, httpfrm, &MyHttpServerForm::onStarted);
    connect(httpsrv, &MyHttpServer::stoped, httpfrm, &MyHttpServerForm::onStoped);
    connect(httpsrv, &MyHttpServer::errorNotice, httpfrm, &MyHttpServerForm::onErrorNotice);
    connect(this, &MainWindow::closeAll, httpfrm, &MyHttpServerForm::close);
    connect(this, &MainWindow::closeAll, httpsrv, &MyHttpServer::stop);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
