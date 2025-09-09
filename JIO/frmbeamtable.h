#ifndef FRMBEAMTABLE_H
#define FRMBEAMTABLE_H

#include <QWidget>
#include <QIODevice>
#include <QColor>
#include "lib/qcustomplot.h"
#include "lib/qcpitemtriangle.h"

#include "aip.h"

namespace Ui {
class FrmBeamTable;
}

class FrmBeamTable : public QCustomPlot
{
    Q_OBJECT

public:
    explicit FrmBeamTable(AIP::ModuleType moduletype, QWidget *parent = nullptr);
    ~FrmBeamTable();
    void setXaxis(QString label, double min, double max);
    void setYaxis(QString label, double min, double max);
    void setData(QVector<QVector<double>> data);
    void loadData(QString filename);
    void loadCyntecData(QIODevice *filedevice);
    void loadHanwhaData(QIODevice *filedevice);
    void setGridPoints(QVector<QVector<double>> data);
    void setEllipseColor(int id, QColor color);
    void setEllipseColor(QCPItemEllipse *ellipse, QColor color);
    void setEllipseBGColor(int id, QColor bgcolor);
    void setEllipseBGColor(QCPItemEllipse *ellipse, QColor bgcolor);
    void clearEllipseBGColor(int id);
    void setEllipse(int id, QString text, QColor color, QColor bgcolor);
    void clearEllipseSelection();
    void addTriangleTarget(QPointF pos);
public slots:
    void onSelectEllipse(QString id, bool clear=true,  QColor color=Qt::green);
protected:
    void changeEvent(QEvent *e);
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Ui::FrmBeamTable *ui;
    AIP::ModuleType mModuletype;
    // QCustomPlot *plot;
    QVector<QVector<double>> mdata; // [[id, az, el], [id, az, el]...]
    QMap<int, QCPItemEllipse*> mEllipses; // hold Ellipse (circle)
    QMap<int, QCPItemText*> mEllipsesValue; //store text
    QCPItemTriangle *mTriangleTarget; // store target pos
    // TooltipHelper *helper;
    int mMinRow=0;
    int mMaxRow;
};

#endif // FRMBEAMTABLE_H
