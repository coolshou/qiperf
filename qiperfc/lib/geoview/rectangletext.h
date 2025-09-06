#ifndef RECTANGLETEXT_H
#define RECTANGLETEXT_H

#include <QString>
#include <QColor>
#include <QPointF>

#include <rectangle.h>
#include <QGeoView/QGVWidgetText.h>
#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVMap.h>

#include "qgvcolortext.h"

class RectangleText : public Rectangle
{
    Q_OBJECT
public:
    explicit RectangleText(QString label, const QGV::GeoRect& geoRect,
                  QPointF fsize, QColor color, QGVMap *map=nullptr);
    QString getText();
private:
    QString projTooltip(const QPointF& projPos) const override;

private:
    // QGVWidgetText* mColorlabel;
    QGV::GeoRect mGeoRect;
    QString mLabel;
    QGVColorText* mColorlabel;
    QGVMap* mMap;
};

#endif // RECTANGLETEXT_H
