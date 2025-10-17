#include "rectangletext.h"

#include <QDebug>

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF fsize, QColor color, QGVMap *map)
    :Rectangle(geoRect, color), mGeoRect(geoRect), mSize(fsize),
    mLabel(label), mColor(color),
    mMap(map)
{

}

QString RectangleText::getText()
{
    return mLabel;
}

QColor RectangleText::getColor()
{
    return mColor;
}

QPointF RectangleText::getSize()
{
    return mSize;
}

QGV::GeoRect RectangleText::getPos()
{
    return mGeoRect;
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
    // setZValue(10); //this affect all Z-value
    // qDebug() << "["<< mLabel << "] "<< painter->hasClipping();
    QRectF rect = getMap()->getProjection()->geoToProj(mGeoRect);
    // qDebug() << "["<< mLabel << "] drawText:" << rect;
    // QRectF txtrect = rect.translate(0, 5);
    rect.moveTo(rect.x(), rect.y() + 2);
    auto path = QGV::createTextPath(rect.toRect(), mLabel, QFont(), pen.width());
    path = QGV::createTransfromScale(rect.center(), 0.75).map(path);
    painter->drawPath(path); //TODO: let the text path on top of the Rectangle,
}
