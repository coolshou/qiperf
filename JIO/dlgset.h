#ifndef DLGSET_H
#define DLGSET_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class DlgSet;
}

class DlgSet : public QDialog
{
    Q_OBJECT

public:
    enum ControlBy{
        None = 0,
        SSH = 1,
        QIPERFD = 2
    };
    Q_ENUM(ControlBy)
    explicit DlgSet(QWidget *parent = nullptr);
    ~DlgSet();
    void setSSH(QString username, QString password);
    void setWeb(QString username, QString password);
    void setDuration(int duration);
    void setInquireInterval(int interval);
    void setCalc(QString distance, QString group, int kmeansfactor);
    void setIfname(QString apifname, QString clientifname);
signals:
    void updateSetting(QString sshusername, QString sshpassword,
                       QString webusername, QString webpassword,
                       DlgSet::ControlBy control, int duration, int interval);
    void updateCalc(QString distance, QString group, int kmeansfactor);
    void updateIfname(QString apifname, QString clientifname);
protected:
    void changeEvent(QEvent *e);
private slots:
    void onAccepted();
private:
    Ui::DlgSet *ui;
};

#endif // DLGSET_H
