#ifndef DLGAIP_H
#define DLGAIP_H

#include <QDialog>
#include <QJsonObject>
#include <QVector3D>
#include <QSettings>

#include "aip.h"
#include "cyntec.h"

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
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onAccepted();
    void onSelModuleClicked(bool checked);
    void onChangeModule(QString newtext);
    void onPreSetPosTextChanged(QString newtext);
    void onXValueChanged(double value);
    void onYValueChanged(double value);
    void onZValueChanged(double value);
    void onAZValueChanged(int value);

private:
    void loadcfg();
    void savecfg();
    Ui::DlgAIP *ui;
    QSettings *m_cfg;
    int mRow;
    int mCol;
    AIP::ModuleType mModuleType;
    QVector3D mPosOffset; // module center position relative to device center (m)
    int mAZOffset;
};

#endif // DLGAIP_H
