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
    void setCalc(QString distance, QString group, int kmeansfactor);
signals:
    void updateSetting(QString sshusername, QString sshpassword,
                       QString webusername, QString webpassword,
                       ControlBy control, int duration);
    void updateCalc(QString distance, QString group, int kmeansfactor);
protected:
    void changeEvent(QEvent *e);
private slots:
    void onAccepted();
private:
    Ui::DlgSet *ui;
};

#endif // DLGSET_H
