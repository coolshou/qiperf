#include "dlgiperfrestartrule.h"
#include "ui_dlgiperfrestartrule.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>

#include <QDebug>

DlgIperfRestartRule::DlgIperfRestartRule(QSettings *cfg, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgIperfRestartRule), m_cfg(cfg)
{
    ui->setupUi(this);
    initMenu();
    ui->twIperfRule->setColumnWidth(cols::Enable, 15);
    ui->twIperfRule->setColumnWidth(cols::Count, 20);
    connect(ui->twIperfRule, &QTableWidget::customContextMenuRequested, this, &DlgIperfRestartRule::showContextMenu);
    // Connect the itemChanged signal to our slot (this is always needed for item-based checkboxes)
    connect(ui->twIperfRule, &QTableWidget::itemChanged, this, &DlgIperfRestartRule::handleItemChanged);
    //button
    connect(ui->pbOK, &QPushButton::clicked, this, &DlgIperfRestartRule::accept);
    connect(ui->pbCancel, &QPushButton::clicked, this, &DlgIperfRestartRule::reject);
    connect(ui->pbSaveDefault, &QPushButton::clicked, this, &DlgIperfRestartRule::onSaveDefault);
    // load config
    loadcfg(cfg);
    // init data

}

DlgIperfRestartRule::~DlgIperfRestartRule()
{
    delete ui;
}

QJsonObject DlgIperfRestartRule::getJsonCfgObj()
{
    QJsonObject obj;
    obj.insert("normalStop", ui->cbIperfNormalStop->isChecked());
    obj.insert("errorStop", ui->cbIperfErrorStop->isChecked());

    QJsonArray rules;
    QJsonArray arrRule;


    return obj;
}

void DlgIperfRestartRule::loadcfg(QSettings *cfg)
{
    //Load config to UI:
    cfg->beginGroup("IperfRestartOnError");
    ui->cbIperfNormalStop->setChecked(cfg->value("normalStop", false).toBool());
    ui->cbIperfErrorStop->setChecked(cfg->value("errorStop", true).toBool());
    cfg->endGroup();
    cfg->beginGroup("IperfRestartRules");
    QStringList rulekeys = cfg->childKeys();
    if (rulekeys.count()>0){
        QString rule;
        for(const QString& key: rulekeys){
            rule = cfg->value(key).toString();
            qDebug() << "key:" << key << " rule:" << rule;
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(rule.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError){
                QJsonObject data = doc.object();
                QString enable = data.value("enable").toString();
                bool bEnable = false;
                QString keyword = data.value("keyword").toString();
                int count = data.value("count").toInt();
                QString comment = data.value("comment").toString();
                if (enable.startsWith("1")){
                    bEnable = true;
                }
                addRowData(bEnable, keyword, count, comment);
            }else{
                QString err=QString("Wrong format of data: %1").arg(error.errorString());
                QMessageBox::information(this, "ERROR", err);
            }
        }
    }
    cfg->endGroup();
}

void DlgIperfRestartRule::setJsonRules(QJsonObject rules)
{
    qDebug() << "setJsonRules:" << rules;
}

void DlgIperfRestartRule::changeEvent(QEvent *e)
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

void DlgIperfRestartRule::onAddRule(bool checked)
{
    Q_UNUSED(checked)
    int count = ui->twIperfRule->rowCount();
    addRowData(true, "keyword "+ QString::number(count),
               1 , "New detect rule");

}
void DlgIperfRestartRule::addRowData(bool enable, const QString& detect, int count, const QString& comment)
{
    int newRow = ui->twIperfRule->rowCount(); // Get the current number of rows (this will be the index of the new row)
    ui->twIperfRule->insertRow(newRow);       // Insert a new row at the end

    // Column 0: Checkbox
    QTableWidgetItem *checkboxItem = new QTableWidgetItem();
    checkboxItem->setFlags(checkboxItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    if (enable){
        checkboxItem->setCheckState(Qt::Checked);
    }else{
        checkboxItem->setCheckState(Qt::Unchecked);
    }
    ui->twIperfRule->setItem(newRow, cols::Enable, checkboxItem);

    // Column 1: Detect
    QTableWidgetItem *detectItem = new QTableWidgetItem(detect);
    ui->twIperfRule->setItem(newRow, cols::Keyword, detectItem);

    // Column 2: Count
    QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(count));
    ui->twIperfRule->setItem(newRow, cols::Count, countItem);

    // Column 3: Comment
    QTableWidgetItem *commentItem = new QTableWidgetItem(comment);
    ui->twIperfRule->setItem(newRow, cols::Comment, commentItem);

    // Auto-adjust column widths if needed after adding new content
    // tableWidget->resizeColumnsToContents(); // This can be expensive for many rows
    // It's often better to do this once after all initial data is loaded,
    // or rely on stretching headers.
}
void DlgIperfRestartRule::onDelRule(bool checked)
{
    Q_UNUSED(checked)
    QList<QTableWidgetItem*> selectedItems = ui->twIperfRule->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::information(this, "No Selection", "Please select one or more rows to remove.");
        return;
    }

    // Use a QSet to store unique row indices to avoid duplicates
    QSet<int> rowsToRemove;
    foreach (QTableWidgetItem* item, selectedItems) {
        rowsToRemove.insert(item->row());
    }

    // Convert the QSet to a QList and sort in descending order
    QList<int> sortedRows;
    foreach (int row, rowsToRemove) {
        sortedRows.append(row);
    }
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>()); // Sort descending

    // Iterate through the sorted list and remove rows
    for (int row : sortedRows) {
        ui->twIperfRule->removeRow(row);
    }

    // Optional: Clear selection after removal if desired
    ui->twIperfRule->clearSelection();

}
void DlgIperfRestartRule::handleItemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        int row = item->row();
        if (item->checkState() == Qt::Checked) {
            qInfo() << "Row" << row << "checkbox checked!";
            // QString name = tableWidget->item(row, 1)->text();
            // qDebug() << "Name for checked item:" << name;
        } else {
            qInfo() << "Row" << row << "checkbox unchecked.";
        }
    }
}
void DlgIperfRestartRule::initMenu()
{
    m_rmenu = new QMenu(this);
    m_AddAction = new QAction("Add Rule");
    connect(m_AddAction, &QAction::triggered, this, &DlgIperfRestartRule::onAddRule);
    m_DelAction = new QAction("Delete Rules");
    connect(m_DelAction, &QAction::triggered, this, &DlgIperfRestartRule::onDelRule);

    m_rmenu->addAction(m_AddAction);
    m_rmenu->addAction(m_DelAction);
}
void DlgIperfRestartRule::showContextMenu(const QPoint &pos)
{
    // Get the item at the clicked position
    QTableWidgetItem *item = ui->twIperfRule->itemAt(pos);
    // TODO Enable/disable actions based condition
    if (item){
        m_DelAction->setEnabled(true);
    }else{
        m_DelAction->setEnabled(false);
    }
    // if (m_clipboard->text().isEmpty()){
    //     m_pasteAction->setEnabled(false);
    // }else{
    //     // TODO: check clipboard's contain is ok to paste
    //     m_pasteAction->setEnabled(true);
    // }
    // Show the menu at the global position of the mouse click
    // Note: mapToGlobal for QTableWidget uses viewport() coordinates
    m_rmenu->exec(ui->twIperfRule->viewport()->mapToGlobal(pos));
}

void DlgIperfRestartRule::onSaveDefault(bool checked)
{
    //save ui's setting to global config
    Q_UNUSED(checked)
    m_cfg->beginGroup("IperfRestartOnError");
    m_cfg->setValue("normalStop", ui->cbIperfNormalStop->isChecked());
    m_cfg->setValue("errorStop", ui->cbIperfErrorStop->isChecked());
    m_cfg->endGroup();
    m_cfg->beginGroup("IperfRestartRules");
    for (int i=0; i<ui->twIperfRule->rowCount(); i++){
        //string
        QJsonObject ruleobj;
        QString chk="1";
        if (ui->twIperfRule->item(i, cols::Enable)->checkState()== Qt::Unchecked){
            chk="0";
        }
        ruleobj.insert("enable", chk);
        ruleobj.insert("keyword", ui->twIperfRule->item(i, cols::Keyword)->text());
        ruleobj.insert("count", ui->twIperfRule->item(i, cols::Count)->text().toInt());
        ruleobj.insert("comment", ui->twIperfRule->item(i, cols::Comment)->text());
        QJsonDocument doc(ruleobj);
        m_cfg->setValue(QString::number(i), doc.toJson(QJsonDocument::Compact));
    }
    m_cfg->endGroup();
}
