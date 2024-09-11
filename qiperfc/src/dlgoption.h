#ifndef DLGOPTION_H
#define DLGOPTION_H

//#include <QWidget>
#include <QDialog>
#include <QSettings>
namespace Ui {
class DlgOption;
}

class dlgOption : public QDialog
{
    Q_OBJECT

public:
    explicit dlgOption(QSettings *cfg, QWidget *parent = nullptr);
//    explicit FormOption(QSettings *cfg, QStringList interfaces, QWidget *parent = nullptr);
    ~dlgOption() override;
    void loadcfg(QSettings *cfg);
    void updatecfg();
    void setWaitServerReady(int val);
    int getWaitServerReady();
    void setTPsize(int width, int heigth);

signals:
    void ipaddressUpdated(QString ipaddress, int port);

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void onReject();

    void onAccept();

private:
    Ui::DlgOption *ui;
    QSettings *m_cfg;
};

#endif // DLGOPTION_H
