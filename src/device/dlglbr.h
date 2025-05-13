#ifndef DLGLBR_H
#define DLGLBR_H

#include <QDialog>

namespace Ui {
class DlgLBR;
}

class DlgLBR : public QDialog
{
    Q_OBJECT

public:
    explicit DlgLBR(QWidget *parent = nullptr);
    ~DlgLBR();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgLBR *ui;
};

#endif // DLGLBR_H
