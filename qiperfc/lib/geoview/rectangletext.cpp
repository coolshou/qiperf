#include "rectangletext.h"

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF size,  QColor color)
    :Rectangle(geoRect, color)
{
    Q_UNUSED(size)
    mlabel = new QGVWidgetText();
    mlabel->setText(label);
    mlabel->setAnchor(QPoint(0, 0), { Qt::TopEdge });
    // auto base = mMap->getProjection()->geoToProj(pos1);
    // QGV::GeoRect pos = mMap->getProjection()->projToGeo({ base, base + QPointF(size.x(), size.y()) });



}
