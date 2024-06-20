#ifndef DLGRECORD_H
#define DLGRECORD_H

#include <QDialog>

namespace Ui {
class DlgRecord;
}

class DlgRecord : public QDialog
{
    Q_OBJECT

public:
    explicit DlgRecord(QWidget *parent = nullptr);
    ~DlgRecord();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgRecord *ui;
};

#endif // DLGRECORD_H
