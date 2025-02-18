#ifndef TPLEGENDITEM_H
#define TPLEGENDITEM_H

#include <QString>
#include <QPen>
#include "qcustomplot.h"
// #include <QCPAbstractLegendItem>

class TPLegendItem : public QCPAbstractLegendItem
{
public:
    TPLegendItem(QString text, QPen pen, QCPLegend *parent);
protected:
    virtual void draw(QCPPainter *painter) override;
    virtual QSize minimumOuterSizeHint() const override;
private:
    QString mText;
    QPen mPen;
};

#endif // TPLEGENDITEM_H
