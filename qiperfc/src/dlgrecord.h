#ifndef DLGRECORD_H
#define DLGRECORD_H

#include <QDialog>
#include <QFileSystemModel>

#include "codeeditor.h"

namespace Ui {
class DlgRecord;
}

class DlgRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DlgRecord( QWidget *parent = nullptr);
    ~DlgRecord();
    void setRootPath(QString rootpath);
public slots:
    void close();

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *e) override;

private slots:
    void onItemDClicked(QModelIndex idx);

private:
    Ui::DlgRecord *ui;
    QFileSystemModel *m_fileModel;
    QString m_rootpath;
    QMap<QString, CodeEditor*> m_logfiles;

};

#endif // DLGRECORD_H
