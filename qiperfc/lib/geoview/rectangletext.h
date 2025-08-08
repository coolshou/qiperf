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
    RectangleText(QString label, const QGV::GeoRect& geoRect,
                  QPointF size, QColor color, QGVMap *map=nullptr);
    QString getText();
private:
    // QGVWidgetText* mlabel;
    QGVColorText* mlabel;
    QGVMap* mMap;
};

#endif // RECTANGLETEXT_H
