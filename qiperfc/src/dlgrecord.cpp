#include "dlgrecord.h"
#include "ui_dlgrecord.h"

#include <QProcess>
#include <QKeySequence>

#include <QDebug>

DlgRecord::DlgRecord(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgRecord)
{
    ui->setupUi(this);
    connect(ui->pbClose, &QPushButton::clicked, this, &DlgRecord::onClose);
    connect(ui->pbBrowser, &QPushButton::clicked, this, &DlgRecord::onBrowser);

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    m_fileModel->setOptions(QFileSystemModel::DontResolveSymlinks|
                            QFileSystemModel::DontUseCustomDirectoryIcons);

    // connect(m_fileModel, &QFileSystemModel::directoryLoaded, this, &DlgRecord::onRefresh);
    connect(m_fileModel, &QFileSystemModel::directoryLoaded, this, [this](const QString &path){
        if (path == m_rootpath) {
            // Option A: Expand/update the view layout
            ui->tvLogFiles->viewport()->update();
        }
    });
    // connect(m_fileModel, &QFileSystemModel::);
    ui->tvLogFiles->setModel(m_fileModel);
    ui->tvLogFiles->setColumnWidth(0, 400);
    ui->tvLogFiles->setColumnWidth(3, 150);

    connect(ui->tvLogFiles, &QTreeView::doubleClicked, this, &DlgRecord::onItemDClicked); //edit item on double click
    // TooltipEventFilter *m_filter = new TooltipEventFilter(ui->tvLogFiles);
    // connect(m_filter, &TooltipEventFilter::doRefresh, this, &DlgRecord::onRefresh);
    // ui->tvLogFiles->viewport()->installEventFilter(m_filter);
    refreshShortcut=new QShortcut(QKeySequence(Qt::Key_F5), ui->tvLogFiles);
    connect(refreshShortcut, &QShortcut::activated,this,&DlgRecord::onRefresh);
    connect(ui->pbRefresh, &QPushButton::clicked, this, &DlgRecord::onRefresh);
}

DlgRecord::~DlgRecord()
{
    delete ui;
}

void DlgRecord::setRootPath(QString rootpath)
{
    m_rootpath = rootpath;
    onRefresh();
    setWindowTitle(QDir::toNativeSeparators(rootpath));
}

void DlgRecord::onClose()
{
    foreach(auto key, m_logfiles.keys()){
        m_logfiles[key]->hide();
        m_logfiles[key]->close();
        m_logfiles.remove(key);
        QApplication::processEvents(QEventLoop::AllEvents);
    }
    this->accept();
}

void DlgRecord::onClosing(QString filename)
{
    if (m_logfiles.contains(filename)){
        CodeEditor *ce = m_logfiles.value(filename);
        ce->deleteLater();
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
    onClose();
    QDialog::closeEvent(e);
}


void DlgRecord::onItemDClicked(QModelIndex idx)
{
    // Check index validity and ensure click was on column 0
    if (!idx.isValid() || idx.column() != 0)
        return;

    // Get exact absolute path directly from QFileSystemModel
    QString filename = m_fileModel->filePath(idx);

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

void DlgRecord::onRefresh()
{
    QModelIndex rootIdx = m_fileModel->setRootPath("");
    rootIdx = m_fileModel->setRootPath(m_rootpath);
    ui->tvLogFiles->setRootIndex(rootIdx);
}

void DlgRecord::onBrowser(bool checked)
{
    Q_UNUSED(checked)
    QProcess process;
#if defined(Q_OS_WIN)
    QString command = "explorer";
    QStringList arguments;
    arguments << QDir::toNativeSeparators(m_rootpath);
#elif defined(Q_OS_MAC)
    QString command = "open";
    QStringList arguments;
    arguments << m_rootpath;
#else
    QString command = "xdg-open";
    QStringList arguments;
    arguments << m_rootpath;
#endif
    process.startDetached(command, arguments);
}
