#include "myhttpserverform.h"
#include "ui_myhttpserverform.h"

#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

MyHttpServerForm::MyHttpServerForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MyHttpServerForm)
{
    ui->setupUi(this);
    connect(ui->pbStart, &QPushButton::clicked, this, &MyHttpServerForm::onStart);
    connect(ui->leRootPath, &QLineEdit::textChanged, this, &MyHttpServerForm::onRootPathChanged);
    connect(ui->pbSelRootPath, &QPushButton::clicked, this, &MyHttpServerForm::onSelectRootPath);
}

MyHttpServerForm::~MyHttpServerForm()
{
    delete ui;
}

void MyHttpServerForm::onStarted()
{
    updateStatus(true);
}

void MyHttpServerForm::onStoped()
{
    updateStatus(false);
}

void MyHttpServerForm::onSelectRootPath(bool checked)
{
    Q_UNUSED(checked)
    QString directoryPath = QFileDialog::getExistingDirectory(this,                          // Parent widget
        tr("Select Directory"),        // Dialog title
        QDir::homePath(),              // Initial directory
        QFileDialog::ShowDirsOnly      // Options: only show directories
                                      );
    if (!directoryPath.isEmpty()) {
        // qDebug() << "Selected directory path:" << directoryPath;
        // Do something with the selected directory path
        ui->leRootPath->setText(directoryPath);
    }
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
            //TODO: host value from combobox
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
    }else{
        ui->pbStart->setText("Start");
    }
}
