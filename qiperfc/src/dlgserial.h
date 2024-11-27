#ifndef DLGSERIAL_H
#define DLGSERIAL_H

#include <QDialog>

namespace Ui {
class DlgSerial;
}

class DlgSerial : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSerial(QWidget *parent = nullptr);
    ~DlgSerial();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgSerial *ui;
};

#endif // DLGSERIAL_H
