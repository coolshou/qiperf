#ifndef DLGIPERF_H
#define DLGIPERF_H

#include <QDialog>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>

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
    bool add(QString mgr);
    void updateUI();

public slots:
    void ChangeVersion(const QString ver);
    void onAccepted();
    void on_chk_bidir_statech(int state);
    void on_chk_reverse_statech(int state);
protected:
    void changeEvent(QEvent *e) override;
//    void closeEvent(QCloseEvent *event) override;

private:
    Ui::DlgIperf *ui;
    QStringList mgrls; //manager ip address list
};

#endif // DLGIPERF_H
