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
public slots:
    void onUpdateHanwhaBeamTableData(double az, double el, double azBW, double elBW);
    void onUpdateBeamTypes(QStringList beamtypes);
    void onUpdateBeamTypeGroup(QMap<QString, QStringList> data);
    void onRefFileTextChanged(QString newtext);
    void setRefFileName(QString filename);
    void onHanwhaBeamDirectionIDChanged(QString newID);
    void onHanwhaBeamTypeTextChanged(QString newBeamType);
signals:
    void reffilechanged(QString filename);
    void closeall();
protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onSelReffileClicked(bool checked);
    void onHanwhaBeamTableClicked(bool checked);
    void onNewHanwhaBeamTableIDs(QStringList keys);
private:
    void onCloseHanwhaBeamTable(int code);
    void loadcfg();
    void savecfg();
private:
    Ui::DlgHanwha *ui;
    QSettings *m_cfg;
    QString m_oldsavepath;
    Hanwha *mHanwha;
    QMap<QString, QStringList> mHanwhaBeamTypeGroup;
};

#endif // DLGHANWHA_H
