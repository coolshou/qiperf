#ifndef DLGINTBOX_H
#define DLGINTBOX_H

#include <QDialog>

namespace Ui {
class DlgIntBox;
}

class DlgIntBox : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIntBox(QWidget *parent = nullptr);
    ~DlgIntBox();
    int getValue();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgIntBox *ui;
};

#endif // DLGINTBOX_H
