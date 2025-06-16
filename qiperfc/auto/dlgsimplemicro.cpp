#include "dlgsimplemicro.h"
#include "ui_dlgsimplemicro.h"

#include <QMessageBox>
#include <QIcon>
#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>

#include "comm.h"
const QByteArray DlgSimpleMicro::MAGIC_VALUE = ".QIS";
const qint32 DlgSimpleMicro::VERSION = 1;

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
    connect(ui->pbLoad, &QPushButton::clicked, this , &DlgSimpleMicro::onLoad);
    connect(ui->pbSave, &QPushButton::clicked, this , &DlgSimpleMicro::onSave);
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
    // check requirement
    QString outpath = ui->leSavePath->text();
    if (outpath.isEmpty()) {
        QMessageBox::information(this, "ERROR", "Please set save path.");
        ui->leSavePath->setFocus();
        return;
    }
    QStringList sl;
    QString filePath="";
    int col = 0;
    int rowCount = ui->tw->rowCount();
    ui->progressBar->setMaximum(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem *item = ui->tw->item(row, col);
        if (item) {
            filePath = item->text();
            if (!QFile::exists(filePath)) {
                QMessageBox::information(this, "ERROR",
                                         QString("File %1 not exist.").arg(filePath));
                selectRowBySettingCurrentCell(row);
                return;
            }
            sl << filePath;
        }
    }
    //
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

void DlgSimpleMicro::onLoad(bool checked)
{
    Q_UNUSED(checked)
    //TODO load simplemicro config
    QString path;
    if (!m_oldpath.isEmpty()){
        path = m_oldpath;
    }else{
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    QString filename = QFileDialog::getOpenFileName(this, "Load simple auto config file", path, QIPERF_AUTOS_EXT_FILTER);
    if (!filename.isEmpty()){
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            QString err=QString("Could not open file for reading: %1 - %2").arg(filename, file.errorString());
            QMessageBox::information(this, "ERROR", err);
            return;
        }
        QByteArray m_magic;
        QByteArray compressedcfg;

        QDataStream in_lff(&file);
        in_lff >> m_magic;
        in_lff >> m_version;
        if (m_magic.startsWith(MAGIC_VALUE)) {
            in_lff >> compressedcfg;
            QByteArray data = qUncompress(compressedcfg);
            QJsonParseError error;
            QJsonDocument doc=QJsonDocument::fromJson(data, &error);
            if (error.error == QJsonParseError::NoError){
                QJsonObject data = doc.object();
                ui->leSavePath->setText(data.value("Savepath").toString());
                ui->tw->clearContents();
                ui->tw->setRowCount(0);
                QJsonArray arrfiles = data.value("files").toArray();
                for (QJsonArray::const_iterator it=arrfiles.constBegin(); it!=arrfiles.constEnd(); ++it) {
                    QJsonObject fsdata= it->toObject();
                    int newRowIndex = ui->tw->rowCount();
                    ui->tw->insertRow(newRowIndex);
                    ui->tw->setItem(newRowIndex, 0, new QTableWidgetItem(fsdata.value("filename").toString()));
                    ui->tw->setItem(newRowIndex, 1, new QTableWidgetItem(fsdata.value("throughput").toString()));
                    ui->tw->setItem(newRowIndex, 2, new QTableWidgetItem(fsdata.value("lostrate").toString()));
                    ui->tw->setItem(newRowIndex, 3, new QTableWidgetItem(fsdata.value("comment").toString()));
                }

            }else{
                qDebug() << "Wrong format of data: " << error.errorString();
            }

        }else{
            qDebug() << "Wrong format of " << filename;
            file.close();
        }

    }

}

void DlgSimpleMicro::onSave(bool checked)
{
    Q_UNUSED(checked)
    //save simplemicro config
    QString path;
    if (!m_oldpath.isEmpty()){
        path = m_oldpath;
    }else{
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
    QString filename = QFileDialog::getSaveFileName(this, "Save simple auto config file", path, QIPERF_AUTOS_EXT_FILTER);
    if (!filename.isEmpty()){
        QString desiredExtension = QString(".%1").arg(QIPERF_AUTOS_EXT);
        if (!filename.endsWith(desiredExtension, Qt::CaseInsensitive)) {
            filename += desiredExtension;
        }

        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
            QString err=QString("Could not open file for writing: %1 - %2").arg(filename, file.errorString());
            QMessageBox::information(this, "ERROR", err);
            return;
        }

        QJsonObject data;
        data["Savepath"] = ui->leSavePath->text();
        QJsonArray arrfiles;
        int rowCount = ui->tw->rowCount();
        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem *item = ui->tw->item(row, 0);
            if (item) {
                QJsonObject fsdata;
                fsdata["filename"] = item->text();

                QTableWidgetItem *item1 = ui->tw->item(row, 1);
                if (item1){
                    fsdata["throughput"] = item1->text();
                }else{
                    fsdata["throughput"] = "";
                }
                QTableWidgetItem *item2 = ui->tw->item(row, 2);
                if (item2){
                    fsdata["lostrate"] = item2->text();
                }else {
                    fsdata["lostrate"] = "";
                }
                QTableWidgetItem *item3 = ui->tw->item(row, 3);
                if (item3){
                    fsdata["comment"] = item3->text();
                }else {
                    fsdata["comment"] = "";
                }
                arrfiles.append(fsdata);
            }
        }
        data["files"] = arrfiles;
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        QDataStream out(&file);
        out << static_cast<QByteArray>(MAGIC_VALUE);
        out << static_cast<qint32>(VERSION);
        out << static_cast<QByteArray>(qCompress(jsonData, 9));

        file.flush();
        file.close();
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
    int rowidx;
    foreach (QString d, ds){
        rowidx = ui->tw->rowCount();
        ui->tw->insertRow(rowidx);
        ui->tw->setItem(rowidx, 0, new QTableWidgetItem(d));
        ui->tw->setItem(rowidx, 1, new QTableWidgetItem(""));
        ui->tw->setItem(rowidx, 2, new QTableWidgetItem(""));
        ui->tw->setItem(rowidx, 3, new QTableWidgetItem(""));
    }
}

void DlgSimpleMicro::onClear(bool checked)
{
    Q_UNUSED(checked)
    ui->tw->clearContents();
    ui->tw->setRowCount(0);
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
    m_clearAction = new QAction(QIcon(":/clear"),"Clear", this);
    connect(m_clearAction, &QAction::triggered, this, &DlgSimpleMicro::onClear);

    m_rightmenu->addAction(m_insertAction);
    m_rightmenu->addAction(m_copyAction);
    m_rightmenu->addAction(m_pasteAction);
    m_rightmenu->addAction(m_deleteAction);
    m_rightmenu->addSeparator();
    m_rightmenu->addAction(m_clearAction);

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
