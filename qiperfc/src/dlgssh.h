#ifndef DLGSSH_H
#define DLGSSH_H

#include <QDialog>

namespace Ui {
class DlgSSH;
}

class DlgSSH : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSSH(QWidget *parent = nullptr);
    ~DlgSSH();
    QString getManagerIP();
    QString getTargetip();
    int getTargetport();
    QString getSSHCfg();
    QString getLogFilename();
    QString getLogTimeStempFormat();
protected:
    void changeEvent(QEvent *e);
private slots:
    void onSelectLogFile(bool checked);
private:
    Ui::DlgSSH *ui;
    QString oldpath;
};

#endif // DLGSSH_H
