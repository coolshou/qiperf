#include "dlgsimplemicro.h"
#include "ui_dlgsimplemicro.h"

#include <QMessageBox>
#include <QIcon>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>

DlgSimpleMicro::DlgSimpleMicro(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSimpleMicro)
{
    ui->setupUi(this);
    initRightMenu();
    ui->tw->setColumnWidth(1, 100);
    ui->tw->setColumnWidth(2, 100);
    connect(ui->pbStart, &QPushButton::clicked, this , &DlgSimpleMicro::onStart);
    connect(ui->pbStop, &QPushButton::clicked, this , &DlgSimpleMicro::onStop);
    connect(ui->pbSelect, &QPushButton::clicked, this , &DlgSimpleMicro::onSelectSavePath);
    ui->tw->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tw, &QTableWidget::customContextMenuRequested, this, &DlgSimpleMicro::showContextMenu);
    m_clipboard = QApplication::clipboard();
}

DlgSimpleMicro::~DlgSimpleMicro()
{
    delete ui;
}

void DlgSimpleMicro::onStart(bool checked)
{
    Q_UNUSED(checked)
    QString outpath = ui->leSavePath->text();
    if (outpath.isEmpty()) {
        QMessageBox::information(this, "ERROR", "Please set save path.");
        ui->leSavePath->setFocus();
        return;
    }
    QStringList sl;
    // ui->tw->rowCount()
    int col = 0;
    int rowCount = ui->tw->rowCount();
    ui->progressBar->setMaximum(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *item = ui->tw->item(row, col);
        if (item) {
            sl << item->text();
        }
    }
    SimpleWorker *m_sworker = new SimpleWorker(sl, outpath);
    QThread *m_thread = new QThread(this);
    connect(m_sworker, &SimpleWorker::loadfile, this, &DlgSimpleMicro::onLoadFile);
    connect(m_sworker, &SimpleWorker::progress, this, &DlgSimpleMicro::onProgress);
    connect(m_sworker, &SimpleWorker::started, this, &DlgSimpleMicro::onStarted);
    connect(m_sworker, &SimpleWorker::stoped, this, &DlgSimpleMicro::onStoped);
    connect(this, &DlgSimpleMicro::reportTP, m_sworker, &SimpleWorker::setTPLostRate);
    connect(m_thread, &QThread::started, m_sworker, &SimpleWorker::run);
    m_sworker->moveToThread(m_thread);
    connect(m_thread, &QThread::finished, m_sworker, &QObject::deleteLater); // Delete worker when thread finishes
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater); // Delete thread when it finishes

    m_thread->start();
}

void DlgSimpleMicro::onStop(bool checked)
{
    Q_UNUSED(checked)
    qDebug() << "// TODO onStop, user force stop test";

}

void DlgSimpleMicro::onSelectSavePath(bool checked)
{
    Q_UNUSED(checked)
    QString path;
    if (!m_oldpath.isEmpty()){
        path = m_oldpath;
    }else{
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    QString dir = QFileDialog::getExistingDirectory(this, "Select save path", path,
                                      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()){
        ui->leSavePath->setText(dir);
        m_oldpath = dir;
    }
}

void DlgSimpleMicro::onLoadFile(QString idx, QString filename, QString savepath)
{
    emit loadfile(idx, filename, savepath);
}

void DlgSimpleMicro::onUpdateTP(int idx, double value, double lostrate)
{
    qDebug() << "DlgSimpleMicro::onUpdateTP:" << QString::number(value);
    QTableWidgetItem *item= new QTableWidgetItem();
    item->setText(QString::number(value));
    ui->tw->setItem(idx, 1, item);

    if (lostrate>=0){
        QTableWidgetItem *item2= new QTableWidgetItem();
        item2->setText(QString::number(lostrate));
        ui->tw->setItem(idx, 2, item2);
    }
    emit reportTP(idx, value, lostrate);
}

void DlgSimpleMicro::showContextMenu(const QPoint &pos)
{
    // Get the item at the clicked position
    QTableWidgetItem *item = ui->tw->itemAt(pos);
    // TODO Enable/disable actions based condition
    if (item){
        m_copyAction->setEnabled(true);
    }else{
        m_copyAction->setEnabled(false);
    }
    if (m_clipboard->text().isEmpty()){
        m_pasteAction->setEnabled(false);
    }else{
        // TODO: check clipboard's contain is ok to paste
        m_pasteAction->setEnabled(true);
    }
    // Show the menu at the global position of the mouse click
    // Note: mapToGlobal for QTableWidget uses viewport() coordinates
    m_rightmenu->exec(ui->tw->viewport()->mapToGlobal(pos));
}

void DlgSimpleMicro::onInsert(bool checked)
{
    Q_UNUSED(checked)
    int currentRow = ui->tw->currentRow();
    int newRow = (currentRow != -1) ? currentRow + 1 : ui->tw->rowCount();
    ui->tw->insertRow(newRow);
    qInfo() << "Inserted new row at index:" << newRow;
    // You might want to select the new row or make it editable
    ui->tw->setCurrentCell(newRow, 0); // Select the first cell of the new row

}

void DlgSimpleMicro::onDelete(bool checked)
{
    Q_UNUSED(checked)
    QList<QTableWidgetItem*> selectedItems = ui->tw->selectedItems();
    if (!selectedItems.isEmpty()) {
        // Get the row of the first selected item.
        // If selection behavior is SelectRows, selectedItems will contain all items in the selected rows.
        // We'll delete rows based on the unique row numbers of selected items.
        QList<int> rowsToDelete;
        for (QTableWidgetItem *item : selectedItems) {
            int row = item->row();
            if (!rowsToDelete.contains(row)) { // Check for duplicates manually
                rowsToDelete.append(row);
            }
        }
        // Delete rows from bottom to top to avoid index shifting issues
        std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>()); // Sort in descending order

        for (int row : rowsToDelete) {
            ui->tw->removeRow(row);
            qInfo() << "Deleted row:" << row;
        }
    } else {
        QMessageBox::information(this, "Delete Action", "No row selected for deletion.");
    }
}

void DlgSimpleMicro::onCopy(bool checked)
{
    Q_UNUSED(checked)
    QStringList ds;
    QList<QTableWidgetItem*> selectedItems = ui->tw->selectedItems();
    // qDebug() << "selectedItems:" << selectedItems;
    for (QTableWidgetItem *itm: selectedItems){
        ds.append(itm->text());
    }
    m_clipboard->setText(ds.join("\n"));
}

void DlgSimpleMicro::onPaste(bool checked)
{
    Q_UNUSED(checked)
    qDebug() << "TODO onPaste";
    QString d = m_clipboard->text();
    QStringList ds = d.split("\n");

}

void DlgSimpleMicro::onProgress(int value)
{
    // ui->tw->setRangeSelected();
    selectRowBySettingCurrentCell(value-1);
    ui->progressBar->setValue(value);
}

void DlgSimpleMicro::onStarted()
{
    ui->pbLoad->setEnabled(false);
    ui->pbSave->setEnabled(false);
    ui->pbStart->setEnabled(false);
    ui->pbStop->setEnabled(true);
}

void DlgSimpleMicro::onStoped()
{
    ui->pbLoad->setEnabled(true);
    ui->pbSave->setEnabled(true);
    ui->pbStart->setEnabled(true);
    ui->pbStop->setEnabled(false);
}

void DlgSimpleMicro::changeEvent(QEvent *e)
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

void DlgSimpleMicro::initRightMenu()
{
    m_rightmenu= new QMenu();
    m_insertAction = new QAction(QIcon(":/insert"), "Insert", this);
    connect(m_insertAction, &QAction::triggered, this, &DlgSimpleMicro::onInsert);
    m_deleteAction = new QAction(QIcon(":/delete"), "Delete", this);
    connect(m_deleteAction, &QAction::triggered, this, &DlgSimpleMicro::onDelete);
    m_copyAction = new QAction(QIcon(":/copy"), "Copy", this);
    connect(m_copyAction, &QAction::triggered, this, &DlgSimpleMicro::onCopy);
    m_pasteAction = new QAction(QIcon(":/paste"),"Paste", this);
    connect(m_pasteAction, &QAction::triggered, this, &DlgSimpleMicro::onPaste);

    m_rightmenu->addAction(m_insertAction);
    m_rightmenu->addAction(m_copyAction);
    m_rightmenu->addAction(m_pasteAction);
    m_rightmenu->addAction(m_deleteAction);

}

void DlgSimpleMicro::selectRowBySettingCurrentCell(int rowToSelect)
{
    if (rowToSelect >= 0 && rowToSelect < ui->tw->rowCount()) {
        // Clear any existing selections first (optional, but good practice)
        ui->tw->clearSelection();

        // Set the current cell to any cell in the desired row.
        // The column index (0 in this case) doesn't matter if SelectRows is active.
        ui->tw->setCurrentCell(rowToSelect, 0); // row, column
        qInfo() << "Row" << rowToSelect << "selected by setting current cell.";
    } else {
        qWarning() << "Invalid row index to select:" << rowToSelect;
    }
}
