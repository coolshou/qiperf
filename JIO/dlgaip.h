#ifndef DLGAIP_H
#define DLGAIP_H

#include <QDialog>
#include <QJsonObject>
#include <QVector3D>
#include <QSettings>

#include "aip.h"
#include "cyntec.h"
#include "hanwha.h"
#include "frmbeamtable.h"

namespace Ui {
class DlgAIP;
}

class DlgAIP : public QDialog
{
    Q_OBJECT

public:
    explicit DlgAIP(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgAIP();
    AIP::ModuleType getModuleType();
    void loadData(QString sdata);
    void loadData(QJsonObject data);
    QJsonObject getData();
    void setPosOffset(float xpos, float ypos, float zpos);
    void setRowCol(int row, int col);
signals:
    void updateData(int row, int col, QJsonObject data);
    void updateModelType(int row, int col, QString smodel);

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onAccepted();
    void onSelReffileClicked(bool checked);
    void onChangeModule(QString newtext);
    void onRefFileTextChanged(QString newtext);
    void onPreSetPosTextChanged(QString newtext);
    void onXValueChanged(double value);
    void onYValueChanged(double value);
    void onZValueChanged(double value);
    void onCyntecBeamFactorIDChanged(QString newBeamFactorID);
    void onCyntecElementMapChanged(QString newElementMap);
    void onCyntecBeamTableIDChanged(QString newBeamTableID);
    void onHanwhaBeamTableIDChanged(QString newBeamTableID);
    void onNewCyntecBeamFactorIDs(QStringList keys);
    void onNewCyntecBeamTableIDs(QStringList keys);
    void onNewHanwhaBeamTableIDs(QStringList keys);
    void onUpdateCynteBeamFactorData(QString elementMap, int attDb, double azBW, double elBW);
    void onUpdateCyntecBeamTableData(double az, double el, double azBW, double elBW);
    void onCyntecBeamTableClicked(bool checked);
    void onHanwhaBeamTableClicked(bool checked);
    void onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW);

private:
    void getCyntecBeamFactorDatas(QString beamFactorID);
    void getCyntecBeamTableDatas(QString beamTableID);
    void onCloseHanwhaBeamTable(int code);
    void loadcfg();
    void savecfg();
    Ui::DlgAIP *ui;
    QSettings *m_cfg;
    int mRow;
    int mCol;
    AIP::ModuleType mModuleType;
    QVector3D mPosOffset; // module center position relative to device center (m)
    QString m_oldsavepath;
    Cyntec *mCyntec;
    Hanwha *mHanwha;
};

#endif // DLGAIP_H
