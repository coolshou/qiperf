#ifndef QGVCOLORTEXT_H
#define QGVCOLORTEXT_H

#include "QGeoView/QGVWidgetText.h"
#include "QGeoView/QGVMap.h"
#include <QPainterPath>
#include <QPainter>
#include <QFontMetrics>

class QGVColorText : public QGVWidgetText
{
    Q_OBJECT
public:
    explicit QGVColorText(const QGV::GeoPos &geoPos, const QSizeF &size);
    void setGeometry(const QGV::GeoPos& geoPos, const QSizeF& size = QSizeF());
    void setGeometry(const QPointF& projPos, const QSizeF& size = QSizeF());

protected:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const;
    void projPaint(QPainter* painter);

private:
    void calculateGeometry();
    QGV::GeoPos mGeoPos;
    QPointF mProjPos;
    QSizeF mSize;
    QRectF mProjRect;
    // QFontMetrics *metrics;
};

#endif // QGVCOLORTEXT_H
