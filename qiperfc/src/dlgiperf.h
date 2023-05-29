#ifndef DLGIPERF_H
#define DLGIPERF_H

#include <QDialog>

namespace Ui {
class DlgIperf;
}

class DlgIperf : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIperf(QWidget *parent = nullptr);
    ~DlgIperf();
    QString getJsonCfg();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgIperf *ui;
};

#endif // DLGIPERF_H
