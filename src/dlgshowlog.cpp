#include "dlgshowlog.h"
#include "ui_dlgshowlog.h"
#include <QDir>
#include <QMessageBox>

#include <QDebug>

DlgShowLog::DlgShowLog(QWidget *parent):
    QDialog(parent),
    ui(new Ui::DlgShowLog)
{
    init();
}

DlgShowLog::DlgShowLog(const QString &filePath, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgShowLog)
{
    if (!filePath.isEmpty()){
        setLogFile(filePath);
    }
    init();
}

DlgShowLog::~DlgShowLog()
{
    delete ui;
}

void DlgShowLog::setLogFile(const QString &filePath)
{
    // TODO: we do not have right to read /tmp/qiperf/ folder, require
    if (!filePath.isEmpty()){
        m_filewatcher= new FileWatcher(filePath);
        connect(m_filewatcher, &FileWatcher::onNewLine, this, &DlgShowLog::appendNewLine);
        setWindowTitle(QDir::toNativeSeparators(filePath));
    }else{
        qDebug() << "log file not specify:";
    }
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

void DlgShowLog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F && event->modifiers() == Qt::ControlModifier){
        //ctrl + F
        ui->w_find->setVisible(true);
        ui->le_find->setText(ui->te_log->textCursor().selectedText());
        ui->le_find->setFocus();
    }else if (event->key() == Qt::Key_Escape){
        ui->w_find->setVisible(false);
    }else{
        QDialog::keyPressEvent(event);
    }
}

void DlgShowLog::onClear(bool checked)
{
    Q_UNUSED(checked)
    ui->te_log->clear();
}

void DlgShowLog::init()
{
    ui->setupUi(this);
    ui->w_find->setVisible(false);

    connect(ui->pbClear, &QPushButton::clicked, this, &DlgShowLog::onClear);
    connect(ui->pbClose, &QPushButton::clicked, this, &DlgShowLog::close);
    connect(ui->le_find, &QLineEdit::returnPressed, this, &DlgShowLog::doFind);
}

void DlgShowLog::appendNewLine(QString line)
{
    ui->te_log->append(line);
    if (ui->cb_scrollbuttom->isChecked()){
        ui->te_log->moveCursor(QTextCursor::EndOfLine);

    }
}

void DlgShowLog::doFind()
{
    QString searchString = ui->le_find->text();
    if (searchString.length()>0){
        QTextDocument::FindFlags options;
        if (ui->cb_CaseSensitive->isChecked()){
            options = QTextDocument::FindCaseSensitively;
        }

        if (!ui->te_log->find(searchString, options)) {
            ui->te_log->moveCursor(QTextCursor::Start);
            if (!ui->te_log->find(searchString, options))
            {
                QMessageBox::information(this, "Find", "The string \"" + searchString + "\" was not found.");
            }
        }
    }
    ui->w_find->setVisible(false);
}
