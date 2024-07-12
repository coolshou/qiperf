#include "dlgrecord.h"
#include "ui_dlgrecord.h"

#include <QAbstractItemModel>
#include "codeeditor.h"

#include <QDebug>

DlgRecord::DlgRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRecord)
{
    ui->setupUi(this);
    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    ui->tvLogFiles->setModel(m_fileModel);
    connect(ui->tvLogFiles, &QTreeView::doubleClicked, this, &DlgRecord::onItemDClicked); //edit item on double click
}

DlgRecord::~DlgRecord()
{
    delete ui;
}

void DlgRecord::setRootPath(QString rootpath)
{
    m_rootpath = rootpath;
    m_fileModel->setRootPath(rootpath);
    QModelIndex idx = m_fileModel->index(m_fileModel->rootPath());
    ui->tvLogFiles->setRootIndex(idx);
    ui->tvLogFiles->setColumnWidth(0, 400);
    ui->tvLogFiles->setColumnWidth(3, 150);
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


void DlgRecord::onItemDClicked(QModelIndex idx)
{
    auto itm = m_fileModel->itemData(idx);
    if (!itm.isEmpty()){
        QString filename = m_rootpath + QDir::separator() + itm[0].toString();
        qDebug() << "onItemDClicked: " << filename;
        CodeEditor *ce=new CodeEditor();
        ce->load(filename);
        ce->show();
    }
//    ce.
}
