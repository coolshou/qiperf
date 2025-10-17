#ifndef RECTANGLETEXT_H
#define RECTANGLETEXT_H

#include <QString>
#include <QColor>
#include <QPointF>

#include <rectangle.h>
#include <QGeoView/QGVWidgetText.h>
#include <QGeoView/QGVGlobal.h>
#include <QGeoView/QGVMap.h>

#include <QPainter>

class RectangleText : public Rectangle
{
    Q_OBJECT
public:
    explicit RectangleText(QString label, const QGV::GeoRect& geoRect,
                  QPointF fsize, QColor color, QGVMap *map=nullptr);
    QString getText();
    QColor getColor();
    QPointF getSize();
    QGV::GeoRect getPos();
private:
    QString projTooltip(const QPointF& projPos) const override;
    void projPaint(QPainter* painter) override;
    void drawText(QPainter* painter);
private:
    QGV::GeoRect mGeoRect;
    QPointF mSize;
    QString mLabel;
    QColor mColor;
    QGVMap* mMap;
};

#endif // RECTANGLETEXT_H
