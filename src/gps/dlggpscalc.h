#ifndef DLGGPSCALC_H
#define DLGGPSCALC_H

#include <QDialog>

namespace Ui {
class DlgGpsCalc;
}

class DlgGpsCalc : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGpsCalc(QWidget *parent = nullptr);
    ~DlgGpsCalc();

protected:
    void changeEvent(QEvent *e);

private slots:
    void onTaipei101SkyTree(bool checked);
    void onCalcCliecked(bool checked);
    void onPosCliecked(bool checked);

private:
    Ui::DlgGpsCalc *ui;
};

#endif // DLGGPSCALC_H
