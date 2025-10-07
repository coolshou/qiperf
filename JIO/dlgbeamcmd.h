#ifndef DLGBEAMCMD_H
#define DLGBEAMCMD_H

#include <QDialog>
#include <QString>
#include <QTextEdit>

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
    void clearCM();
    void onAddBeamIDCmd(QString cmd);
    void onAddCMBeamIDCmd(QString name, QString cmd);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgBeamCmd *ui;
    QMap<QString, QTextEdit*> mCMCmds; // tab name, textedit to hold command
};

#endif // DLGBEAMCMD_H
