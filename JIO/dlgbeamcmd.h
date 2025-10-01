#ifndef DLGBEAMCMD_H
#define DLGBEAMCMD_H

#include <QDialog>
#include <QString>

namespace Ui {
class DlgBeamCmd;
}

class DlgBeamCmd : public QDialog
{
    Q_OBJECT

public:
    explicit DlgBeamCmd(QWidget *parent = nullptr);
    ~DlgBeamCmd();
public slots:
    void clear();
    void onAddBeamIDCmd(QString cmd);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgBeamCmd *ui;
};

#endif // DLGBEAMCMD_H
