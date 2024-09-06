#include "dlgrecord.h"
#include "ui_dlgrecord.h"

#include <QDebug>

DlgRecord::DlgRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRecord)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DlgRecord::close);

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    ui->tvLogFiles->setModel(m_fileModel);
    connect(ui->tvLogFiles, &QTreeView::doubleClicked, this, &DlgRecord::onItemDClicked); //edit item on double click
    // TooltipEventFilter *m_filter = new TooltipEventFilter(ui->tvLogFiles);
    // connect(m_filter, &TooltipEventFilter::doRefresh, this, &DlgRecord::onRefresh);
    // ui->tvLogFiles->viewport()->installEventFilter(m_filter);
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
    setWindowTitle(QDir::toNativeSeparators(rootpath));
}

void DlgRecord::close()
{
    foreach(auto key, m_logfiles.keys()){
        m_logfiles[key]->hide();
        m_logfiles[key]->close();
        m_logfiles.remove(key);
        QApplication::processEvents(QEventLoop::AllEvents);
    }

}

void DlgRecord::onClosing(QString filename)
{
    if (m_logfiles.contains(filename)){
        m_logfiles.remove(filename);
    }
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

void DlgRecord::closeEvent(QCloseEvent *e)
{
    close();
    QDialog::closeEvent(e);
}


void DlgRecord::onItemDClicked(QModelIndex idx)
{
    if (idx.column()==0){
        auto itm = m_fileModel->itemData(idx);
        if (!itm.isEmpty()){
            QString filename = m_rootpath + QDir::separator() + itm[0].toString();
            if (!m_logfiles.contains(filename)){
                m_logfiles[filename] = new CodeEditor();
                connect(m_logfiles[filename], &CodeEditor::Closing, this, &DlgRecord::onClosing);
                m_logfiles[filename]->load(filename);
                m_logfiles[filename]->show();
            }else{
                m_logfiles[filename]->setWindowState((m_logfiles[filename]->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                m_logfiles[filename]->raise();
            }
        }
    }
}

void DlgRecord::onRefresh()
{
    qDebug() << "onRefresh";
    m_fileModel->setRootPath(m_rootpath);
}
