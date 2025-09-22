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
    explicit RectangleText(QString label, const QGV::GeoRect& geoRect,
                  QPointF fsize, QColor color, QGVMap *map=nullptr);
    QString getText();
private:
    QString projTooltip(const QPointF& projPos) const override;
    void projPaint(QPainter* painter) override;
    void drawText(QPainter* painter);
private:
    QGV::GeoRect mGeoRect;
    QString mLabel;
    QColor mColor;
    QGVMap* mMap;
};

#endif // RECTANGLETEXT_H
