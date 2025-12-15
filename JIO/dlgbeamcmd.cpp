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
    ui->teUCI->clear();
    clearCM();
}

void DlgBeamCmd::clearCM()
{
    foreach (auto key, mClientCmds.keys()) {
        mClientCmds.value(key)->clear();
    }
    foreach (auto key, mClientUCICmds.keys()) {
        mClientUCICmds.value(key)->clear();
    }
}

void DlgBeamCmd::onAddBeamIDCmd(QString cmd, bool bUCI)
{
    if (bUCI){
        //TODO : uci command
        ui->teUCI->append(cmd);
    }else{
        ui->textEdit->append(cmd);
    }


}

void DlgBeamCmd::onAddClientBeamIDCmd(QString name, QString cmd)
{
    QTextEdit *ed=nullptr;
    QTextEdit *edUCI=nullptr;
    if (!mClientCmds.contains(name)){
        QWidget *page = new QWidget();
        ed = new QTextEdit(page);
        edUCI = new QTextEdit(page);
        QHBoxLayout *layout = new QHBoxLayout(page);
        layout->addWidget(ed);
        layout->addWidget(edUCI);
        layout->setStretch(0,1);
        page->setLayout(layout);
        ui->tabWidget->addTab(page, name);
        mClientCmds.insert(name, ed);
        mClientUCICmds.insert(name, edUCI);
    }else{
        ed = mClientCmds.value(name);
        edUCI = mClientUCICmds.value(name);
    }
    ed->append(cmd);
    // TODO client UCI command
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
