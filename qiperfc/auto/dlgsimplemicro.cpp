#include "dlgsimplemicro.h"
#include "ui_dlgsimplemicro.h"

#include <QMessageBox>
#include <QIcon>

DlgSimpleMicro::DlgSimpleMicro(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgSimpleMicro)
{
    ui->setupUi(this);
    initRightMenu();
    ui->tw->setColumnWidth(1, 100);
    ui->tw->setColumnWidth(2, 100);
    connect(ui->pbStart, &QPushButton::clicked, this , &DlgSimpleMicro::onStart);
    ui->tw->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tw, &QTableWidget::customContextMenuRequested, this, &DlgSimpleMicro::showContextMenu);

}

DlgSimpleMicro::~DlgSimpleMicro()
{
    delete ui;
}

void DlgSimpleMicro::onStart(bool checked)
{
    Q_UNUSED(checked)
    QStringList sl;
    sl << "";
    SimpleWorker *m_sworker = new SimpleWorker(sl);
    QThread *m_thread = new QThread();
    connect(m_sworker, &SimpleWorker::loadfile, this, &DlgSimpleMicro::onLoadFile);
    connect(m_thread, &QThread::started, m_sworker, &SimpleWorker::run);
    m_sworker->moveToThread(m_thread);

    m_thread->start();
}

void DlgSimpleMicro::onLoadFile(QString idx, QString filename)
{
    emit loadfile(idx, filename);
}

void DlgSimpleMicro::showContextMenu(const QPoint &pos)
{
    // Get the item at the clicked position
    QTableWidgetItem *item = ui->tw->itemAt(pos);
    // TODO Enable/disable actions based condition

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
    qDebug() << "TODO onDelete";
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
    qDebug() << "TODO onCopy";
}

void DlgSimpleMicro::onPaste(bool checked)
{
    Q_UNUSED(checked)
    qDebug() << "TODO onPaste";
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
    QAction *m_insertAction = new QAction(QIcon(":/insert"), "Insert", this);
    connect(m_insertAction, &QAction::triggered, this, &DlgSimpleMicro::onInsert);
    QAction *m_deleteAction = new QAction(QIcon(":/delete"), "Delete", this);
    connect(m_deleteAction, &QAction::triggered, this, &DlgSimpleMicro::onDelete);
    QAction *m_copyAction = new QAction(QIcon(":/copy"), "Copy", this);
    connect(m_copyAction, &QAction::triggered, this, &DlgSimpleMicro::onCopy);
    QAction *m_pasteAction = new QAction(QIcon(":/paste"),"Paste", this);
    connect(m_pasteAction, &QAction::triggered, this, &DlgSimpleMicro::onPaste);

    m_rightmenu->addAction(m_insertAction);
    m_rightmenu->addAction(m_copyAction);
    m_rightmenu->addAction(m_pasteAction);
    m_rightmenu->addAction(m_deleteAction);

}
