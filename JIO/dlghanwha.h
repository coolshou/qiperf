#ifndef DLGHANWHA_H
#define DLGHANWHA_H

#include <QDialog>
#include "hanwha.h"
#include "frmbeamtable.h"

namespace Ui {
class DlgHanwha;
}

class DlgHanwha : public QDialog
{
    Q_OBJECT

public:
    explicit DlgHanwha(Hanwha *hanwha, QWidget *parent = nullptr);
    ~DlgHanwha();
public slots:
    void onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW);
    void onUpdateBeamTypes(QStringList beamtypes);
    void onUpdateBeamTypeGroup(QMap<QString, QStringList> data);
    void onRefFileTextChanged(QString newtext);
    void setRefFileName(QString filename);
signals:
    void reffilechanged(QString filename);
protected:
    void changeEvent(QEvent *e);
private slots:
    void onSelReffileClicked(bool checked);
    void onHanwhaBeamTableIDChanged(QString newBeamTableID);
    void onHanwhaBeamTypeTextChanged(QString newBeamType);
    void onHanwhaBeamTableClicked(bool checked);
    void onNewHanwhaBeamTableIDs(QStringList keys);
private:
    void onCloseHanwhaBeamTable(int code);
private:
    Ui::DlgHanwha *ui;
    QString m_oldsavepath;
    Hanwha *mHanwha;
    QMap<QString, QStringList> mHanwhaBeamTypeGroup;
};

#endif // DLGHANWHA_H
