#include "myhttpserverform.h"
#include "ui_myhttpserverform.h"

#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include "../src/myfunc.h"

MyHttpServerForm::MyHttpServerForm(QSettings *cfg, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyHttpServerForm), mCfg(cfg)
{
    ui->setupUi(this);
    connect(ui->pbStart, &QPushButton::clicked, this, &MyHttpServerForm::onStart);
    connect(ui->leRootPath, &QLineEdit::textChanged, this, &MyHttpServerForm::onRootPathChanged);
    connect(ui->pbSelRootPath, &QPushButton::clicked, this, &MyHttpServerForm::onSelectRootPath);
    connect(ui->pbReflash, &QPushButton::clicked, this, &MyHttpServerForm::onReflash);
    onReflash(false);
    LoadCfg();
}

MyHttpServerForm::~MyHttpServerForm()
{
    delete ui;
}

void MyHttpServerForm::LoadCfg(QSettings *cfg)
{
    // QSettings *mcfg=nullptr;
    if (cfg != nullptr){
        mCfg = cfg;
    }
    if (mCfg != nullptr){
        mCfg->beginGroup("httpd");
        mOldRootPath = mCfg->value("oldrootpath", QDir::homePath()).toString();
        mCfg->endGroup();
        ui->leRootPath->setText(mOldRootPath);
    }
}

void MyHttpServerForm::SaveCfg()
{
    if (mCfg){
        mCfg->beginGroup("httpd");
        mCfg->setValue("oldrootpath", mOldRootPath);
        mCfg->endGroup();
    }
}

void MyHttpServerForm::onStarted()
{
    updateStatus(true);
    QString host = ui->cbHostAddress->currentText();
    quint16 port = ui->sbPort->value();

    ui->textEdit->clear();
    QString msg = QString("http://%1:%2").arg(host, QString::number(port));
    ui->textEdit->append("start listen on "+ msg);
    ui->textEdit->append("================================================================================");
    //list files
    QDir dir(ui->leRootPath->text());
    QStringList files = dir.entryList(QDir::Files);
    for (const QString &file : files) {
        ui->textEdit->append(msg+"/"+file);
    }
    ui->textEdit->append("================================================================================");
}

void MyHttpServerForm::onStoped()
{
    updateStatus(false);
    ui->textEdit->clear();
    ui->textEdit->append("httpd Stoped");
}

void MyHttpServerForm::onErrorNotice(QString err)
{
    ui->textEdit->append(err);
    updateStatus(false);
}

void MyHttpServerForm::onSelectRootPath(bool checked)
{
    Q_UNUSED(checked)
    QString path;
    if (!mOldRootPath.isEmpty()){
        path = mOldRootPath;
    }else {
        path = QDir::homePath();
    }
    QString directoryPath = QFileDialog::getExistingDirectory(this,                          // Parent widget
        tr("Select Directory"),        // Dialog title
        path,              // Initial directory
        QFileDialog::ShowDirsOnly      // Options: only show directories
                                      );
    if (!directoryPath.isEmpty()) {
        mOldRootPath = directoryPath;
        // qDebug() << "Selected directory path:" << directoryPath;
        // Do something with the selected directory path
        ui->leRootPath->setText(directoryPath);
    }
}

void MyHttpServerForm::onReflash(bool checked)
{
    Q_UNUSED(checked)
    QStringList ls = MyFunc::getAllIPAddress();
    // qDebug() << "onReflash:" << ls.join(",");
    ui->cbHostAddress->clear();
    ui->cbHostAddress->insertItem(0, "Any");
    ui->cbHostAddress->insertItems(1, ls);
}

void MyHttpServerForm::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    SaveCfg();
}

void MyHttpServerForm::onStart(bool checked)
{
    // Q_UNUSED(checked)
    if (checked){
        quint16 port = static_cast<quint16>(ui->sbPort->value());
        QString path = ui->leRootPath->text();
        //check path exist
        if (!QDir(path).exists()){
            QMessageBox::information(this, "Root PAth", "http root path not exist!");
            ui->leRootPath->setFocus();
            return;
        }

        QHostAddress host = QHostAddress::Any;
        if (ui->cbHostAddress->currentIndex()!=0){
            host.setAddress(ui->cbHostAddress->currentText());
        }

        emit sigStart(port, path, host);
    }else {
        emit sigStop();
    }
}

void MyHttpServerForm::onRootPathChanged(QString path)
{
    emit sigRootPathChange(path);
}

void MyHttpServerForm::updateStatus(bool started)
{
    if (started){
        ui->pbStart->setText("Stop");
        ui->leRootPath->setEnabled(false);
        ui->cbHostAddress->setEnabled(false);
        ui->sbPort->setEnabled(false);
        ui->pbSelRootPath->setEnabled(false);

    }else{
        ui->pbStart->setText("Start");
        if (ui->pbStart->isChecked())
        {
            ui->pbStart->setChecked(false);
        }
        ui->leRootPath->setEnabled(true);
        ui->cbHostAddress->setEnabled(true);
        ui->sbPort->setEnabled(true);
        ui->pbSelRootPath->setEnabled(true);
    }
}
