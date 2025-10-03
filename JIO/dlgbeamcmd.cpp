#include "dlgbeamcmd.h"
#include "ui_dlgbeamcmd.h"

DlgBeamCmd::DlgBeamCmd(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgBeamCmd)
{
    ui->setupUi(this);
}

DlgBeamCmd::~DlgBeamCmd()
{
    delete ui;
}

void DlgBeamCmd::clear()
{
    ui->textEdit->clear();
    foreach (auto key, mCMCmds.keys()) {
        mCMCmds.value(key)->clear();
    }
}

void DlgBeamCmd::onAddBeamIDCmd(QString cmd)
{
    ui->textEdit->append(cmd);
}

void DlgBeamCmd::onAddCMBeamIDCmd(QString name, QString cmd)
{
    QTextEdit *ed=nullptr;
    if (!mCMCmds.contains(name)){
        QWidget *page = new QWidget();
        ed = new QTextEdit(page);
        QVBoxLayout *layout = new QVBoxLayout(page);
        layout->addWidget(ed);
        page->setLayout(layout);
        ui->tabWidget->addTab(page, name);
        mCMCmds.insert(name, ed);
    }else{
        ed = mCMCmds.value(name);
    }
    ed->append(cmd);
}

void DlgBeamCmd::changeEvent(QEvent *e)
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
