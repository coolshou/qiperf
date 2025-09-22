#ifndef DLGCYNTEC_H
#define DLGCYNTEC_H

#include <QDialog>
#include "cyntec.h"
#include "frmbeamtable.h"

namespace Ui {
class DlgCyntec;
}

class DlgCyntec : public QDialog
{
    Q_OBJECT

public:
    explicit DlgCyntec(QSettings *cfg, Cyntec *cyntec, QWidget *parent = nullptr);
    ~DlgCyntec();
    QString getCyntecBeamType();
    FrmBeamTable *getBeamTable();
public slots:
    void onNewCyntecBeamFactorIDs(QStringList keys);
    void onNewCyntecBeamTableIDs(QStringList keys);
    void onUpdateCynteBeamFactorData(QString elementMap, double attDb, double azBW, double elBW);
    void onUpdateCyntecBeamTableData(double az, double el, double azBW, double elBW);
    void onRefFileTextChanged(QString newtext);
    void setRefFileName(QString filename);
    void onUpdateBeamTypes(QStringList beamtypes);
    void onUpdateBeamTypeGroup(QMap<QString, QList<int>> data);
    void onUpdateBeamFactorSupport(QMap<QString, QList<int>> data);
    void onCyntecBeamTypeTextChanged(QString newBeamType);
signals:
    void reffilechanged(QString filename);
    void closeall();
    void SelectEllipse(QString id, bool clear, QColor color);
protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onSelReffileClicked(bool checked);
    void onCyntecBeamTableClicked(bool checked);
    void onCyntecBeamFactorIDChanged(QString newBeamFactorID);
    void onCyntecBeamTableIDChanged(QString newBeamTableID);
    void onCyntecElementMapChanged(QString newElementMap);
    void onSelectAroundID(bool checked);
    void onAddTriangle(bool checked);
private:
    void getCyntecBeamFactorDatas(QString beamFactorID);
    void getCyntecBeamTableDatas(QString beamTableID);
    void updateCyntecBeamTableID(QString beamType, QString beamFactorID);
    void loadcfg();
    void savecfg();
    void AddTriangle(double xpos, double ypos, double size);

private:
    Ui::DlgCyntec *ui;
    QSettings *m_cfg;
    QString m_oldsavepath;
    Cyntec *mCyntec;
    QMap<QString, QList<int>> mCyntecBeamTypeGroup;
    QMap<QString, QList<int>> mCyntecBeamFactorSupport;
    FrmBeamTable *nBeamT; //Narrow
    FrmBeamTable *sBeamT; //Spoiled
    FrmBeamTable *tBeamT; //TRi
};

#endif // DLGCYNTEC_H
