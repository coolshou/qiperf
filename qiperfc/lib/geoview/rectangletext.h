#ifndef RECTANGLETEXT_H
#define RECTANGLETEXT_H

#include <QString>
#include <QColor>
#include <QPointF>

#include <rectangle.h>
#include <QGeoView/QGVWidgetText.h>
#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVMap.h>

class RectangleText : public Rectangle
{
    Q_OBJECT
public:
    RectangleText(QString label, const QGV::GeoRect& geoRect,
                  QPointF size, QColor color);
private:
    QGVWidgetText* mlabel;

};

#endif // RECTANGLETEXT_H
