#ifndef DLGRECORD_H
#define DLGRECORD_H

#include <QDialog>
#include <QFileSystemModel>

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

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void onItemDClicked(QModelIndex idx);

private:
    Ui::DlgRecord *ui;
    QFileSystemModel *m_fileModel;
    QString m_rootpath;
};

#endif // DLGRECORD_H
