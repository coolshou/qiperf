#include "rectangletext.h"

#include <QDebug>

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF size,  QColor color, QGVMap *map)
    :Rectangle(geoRect, color), mMap(map)
{
    Q_UNUSED(size)
    mlabel = new QGVWidgetText();
    mlabel->setText(label);
    qDebug() << "["<<  label << "]latitude:" << geoRect.topRight().latToString()
             << "longitude:" << geoRect.topRight().lonToString();
    //TODO: following will fix on a position, the position should next to the Rectangle
    /*
    mlabel = new QGVWidgetText();
    mlabel->setMap(map);
    mlabel->setText(label);
    mlabel->setAnchor(QPoint(geoRect.topRight().latitude(),
                             geoRect.topRight().longitude()), { Qt::TopEdge });

    if (mMap){
        mMap->addWidget(mlabel);
    }
    */
    // auto base = mMap->getProjection()->geoToProj(pos1);
    // QGV::GeoRect pos = mMap->getProjection()->projToGeo({ base, base + QPointF(size.x(), size.y()) });



}

QString RectangleText::getText()
{
    return mlabel->getText();
}
