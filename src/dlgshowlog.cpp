#include "dlgshowlog.h"
#include "ui_dlgshowlog.h"

DlgShowLog::DlgShowLog(const QString &filePath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgShowLog)
{
    ui->setupUi(this);
    // TODO: we do not have right to read /tmp/qiperf/ folder, require
    //m_filewatcher= new FileWatcher(filePath);
    //connect(m_filewatcher, &FileWatcher::onNewLine, this, &DlgShowLog::appendNewLine);
    setWindowTitle(filePath);
    connect(ui->pbClear, &QPushButton::clicked, this, &DlgShowLog::onClear);
    connect(ui->pbClose, &QPushButton::clicked, this, &DlgShowLog::close);
}

DlgShowLog::~DlgShowLog()
{
    delete ui;
}

void DlgShowLog::changeEvent(QEvent *e)
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

void DlgShowLog::onClear(bool checked)
{
    Q_UNUSED(checked)
    ui->te_log->clear();
}

void DlgShowLog::appendNewLine(QString line)
{
    ui->te_log->append(line);
}
