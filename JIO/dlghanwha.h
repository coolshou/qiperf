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
    explicit DlgHanwha(QSettings *cfg, Hanwha *hanwha, QWidget *parent = nullptr);
    ~DlgHanwha();
    QString getHanwhaBeamType();
    FrmBeamTable *getBeamTable();
    void setBeamTable(FrmBeamTable *beamtable);
public slots:
    void onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW);
    void onUpdateBeamTypes(QStringList beamtypes);
    void onUpdateBeamTypeGroup(QMap<QString, QList<int>> data);
    void onRefFileTextChanged(QString newtext);
    void setRefFileName(QString filename);
    void onHanwhaBeamDirectionIDChanged(QString newBeamTableID);
    void onHanwhaBeamTypeTextChanged(QString newBeamType);
signals:
    void reffilechanged(QString filename);
    void closeall();
    void SelectEllipse(QString id, bool clear, QColor color);
protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onSelReffileClicked(bool checked);
    void onHanwhaBeamTableClicked(bool checked);
    void onNewHanwhaBeamTableIDs(QStringList keys);
    void onSelectAroundID(bool checked);
private:
    void onCloseHanwhaBeamTable(int code);
    void loadcfg();
    void savecfg();
private:
    Ui::DlgHanwha *ui;
    QSettings *m_cfg;
    QString m_oldsavepath;
    Hanwha *mHanwha;
    QMap<QString, QList<int>> mHanwhaBeamTypeGroup;
    FrmBeamTable *nBeamT; //NARROW
    FrmBeamTable *wBeamT; //widebeam
    FrmBeamTable *tBeamT; //TRi
    FrmBeamTable *qBeamT; //quater
};

#endif // DLGHANWHA_H
