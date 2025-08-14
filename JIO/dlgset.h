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
signals:
    void updateSetting(QString sshusername, QString sshpassword,
                       QString webusername, QString webpassword,
                       ControlBy control);
protected:
    void changeEvent(QEvent *e);
private slots:
    void onAccepted();
private:
    Ui::DlgSet *ui;
};

#endif // DLGSET_H
