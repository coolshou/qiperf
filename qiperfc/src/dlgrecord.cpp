#include "dlgrecord.h"
#include "ui_dlgrecord.h"

DlgRecord::DlgRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRecord)
{
    ui->setupUi(this);
    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    ui->tvLogFiles->setModel(m_fileModel);
}

DlgRecord::~DlgRecord()
{
    delete ui;
}

void DlgRecord::setRootPath(QString rootpath)
{
    m_fileModel->setRootPath(rootpath);
    QModelIndex idx = m_fileModel->index(m_fileModel->rootPath());
    ui->tvLogFiles->setRootIndex(idx);

    setWindowTitle(rootpath);
}

void DlgRecord::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
