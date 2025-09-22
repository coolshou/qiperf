#include "rectangletext.h"

#include <QDebug>

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF fsize, QColor color, QGVMap *map)
    :Rectangle(geoRect, color), mGeoRect(geoRect), mLabel(label), mColor(color),
    mMap(map)
{

}

QString RectangleText::getText()
{
    return mColorlabel->getText();
}

QString RectangleText::projTooltip(const QPointF &projPos) const
{   Q_UNUSED(projPos)
    QGV::GeoPos topLeftGeo = mGeoRect.topLeft();
    const QPointF& ppos = getMap()->getProjection()->geoToProj(topLeftGeo);
    auto geo = getMap()->getProjection()->projToGeo(ppos);

    return mLabel + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

void RectangleText::projPaint(QPainter *painter)
{
    // Call the base class implementation
    Rectangle::projPaint(painter);
    drawText(painter);
}

void RectangleText::drawText(QPainter *painter)
{
    QPen pen = QPen(Qt::black);
    pen.setWidth(1);
    pen.setCosmetic(true);
    QBrush brush = QBrush(Qt::black);
    painter->setPen(pen);
    painter->setBrush(brush);
    QRectF rect = getMap()->getProjection()->geoToProj(mGeoRect);
    // qDebug() << "drawText:" << rect;
    auto path = QGV::createTextPath(rect.toRect(), mLabel, QFont(), pen.width());
    path = QGV::createTransfromScale(rect.center(), 0.75).map(path);
    painter->drawPath(path);
}
