#include "rectangletext.h"

#include <QDebug>

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF fsize,  QColor color, QGVMap *map)
    :Rectangle(geoRect, color), mMap(map)
{
    // Q_UNUSED(size)
    // mlabel = new QGVWidgetText();
    qDebug() << "geoRect:"<< geoRect;
             // << " base:topLeft:" << base.topLeft()
             // << " bottomRight:" << base.bottomRight();
    // auto pos = getMap()->
    mlabel = new QGVColorText(QGV::GeoPos(geoRect.latTop(), geoRect.lonLeft()),
                              QSizeF(fsize.x(), fsize.y()));
    mlabel->setMap(map);
    mlabel->setText(label);
    //TODO: following will fix on a position, the position should next to the Rectangle
    // QRect base = mMap->getProjection()->geoToProj(geoRect).toRect();
    // mlabel->setGeometry(QGV::GeoPos(geoRect.latTop(), geoRect.lonLeft()),
    //                     QSizeF(fsize.x(), fsize.y()));
    // mlabel->setAnchor(base.topLeft(),
    //                   {Qt::TopEdge} );
    if (mMap){
        mMap->addWidget(mlabel);
    }
    qDebug() << "["<<  label << "]latitude:" << geoRect.topRight().latToString()
             << "longitude:" << geoRect.topRight().lonToString();
    // auto base = mMap->getProjection()->geoToProj(pos1);
    // QGV::GeoRect pos = mMap->getProjection()->projToGeo({ base, base + QPointF(size.x(), size.y()) });
}

QString RectangleText::getText()
{
    return mlabel->getText();
}
