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
    ~DlgIperf() override;
    QString getJsonCfg();
    void loadJsonCfg(QString jsoncfg);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::DlgIperf *ui;
};

#endif // DLGIPERF_H
