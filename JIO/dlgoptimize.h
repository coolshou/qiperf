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
    void onAddData(QDateTime testtime, int am7beamid,
                 QString cm7name, int cm7beamid);
    void onUpdateSignal(QDateTime testtime, double am7mcs, double am7rssi, double am7snr,
                QString cm7name, double cm7mcs, double cm7rssi, double cm7snr);
    void onUpdateTP(QDateTime testtime, QString cm7name, double ul, double dl);

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
