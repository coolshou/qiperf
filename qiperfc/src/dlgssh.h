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

protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgSSH *ui;
};

#endif // DLGSSH_H
