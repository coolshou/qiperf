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
public slots:
    void onNewCyntecBeamFactorIDs(QStringList keys);
    void onNewCyntecBeamTableIDs(QStringList keys);
    void onUpdateCynteBeamFactorData(QString elementMap, int attDb, double azBW, double elBW);
    void onUpdateCyntecBeamTableData(double az, double el, double azBW, double elBW);
    void onRefFileTextChanged(QString newtext);
    void setRefFileName(QString filename);
signals:
    void reffilechanged(QString filename);
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onSelReffileClicked(bool checked);
    void onCyntecBeamTableClicked(bool checked);
    void onCyntecBeamFactorIDChanged(QString newBeamFactorID);
    void onCyntecBeamTableIDChanged(QString newBeamTableID);
    void onCyntecElementMapChanged(QString newElementMap);
private:
    void getCyntecBeamFactorDatas(QString beamFactorID);
    void getCyntecBeamTableDatas(QString beamTableID);
    void loadcfg();
    void savecfg();

private:
    Ui::DlgCyntec *ui;
    QSettings *m_cfg;
    QString m_oldsavepath;
    Cyntec *mCyntec;
};

#endif // DLGCYNTEC_H
