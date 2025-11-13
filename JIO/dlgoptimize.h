#ifndef DLGOPTIMIZE_H
#define DLGOPTIMIZE_H

#include <QDialog>
#include "optimizemodel.h"

namespace Ui {
class DlgOptimize;
}

class DlgOptimize : public QDialog
{
    Q_OBJECT

public:
    explicit DlgOptimize(QWidget *parent = nullptr);
    ~DlgOptimize();
public slots:
    void onAddData(QDateTime testtime, int apbeamid,
                   QString clientname, int clientbeamid);
    void onUpdateSignal(QDateTime testtime,
                        double apmcs, double aprssi, double apsnr,
                        QString cname,
                        double cmcs, double crssi, double csnr);
    void onUpdateTP(QDateTime testtime, QString cname, double ul, double dl);

protected:
    void changeEvent(QEvent *e);

private:
    void testdata1();
    void testdata2();
    void testdata3();
    void testdata4();
    void testdata5();
    Ui::DlgOptimize *ui;
    OptimizeModel *model;
};

#endif // DLGOPTIMIZE_H
