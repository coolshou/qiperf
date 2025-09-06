#include "rectangletext.h"

#include <QDebug>

RectangleText::RectangleText(QString label, const QGV::GeoRect &geoRect,
                             QPointF fsize,  QColor color, QGVMap *map)
    :Rectangle(geoRect, color), mGeoRect(geoRect), mLabel(label), mMap(map)
{
    // Q_UNUSED(size)
    // mlabel = new QGVWidgetText();
    // auto pos = getMap()->
    mColorlabel = new QGVColorText(QGV::GeoPos(geoRect.latTop(), geoRect.lonLeft()),
                              QSizeF(fsize.x(), fsize.y()));
    mColorlabel->setMap(map);
    mColorlabel->setText(label);
    //TODO: following will fix on a position, the position should next to the Rectangle
    // QRect base = mMap->getProjection()->geoToProj(geoRect).toRect();
    // mlabel->setGeometry(QGV::GeoPos(geoRect.latTop(), geoRect.lonLeft()),
    //                     QSizeF(fsize.x(), fsize.y()));
    // mlabel->setAnchor(base.topLeft(),
    //                   {Qt::TopEdge} );
    if (mMap){
        mMap->addWidget(mColorlabel);
    }
    qDebug() << "["<<  label << "]latitude:" << geoRect.topRight().latToString()
             << "longitude:" << geoRect.topRight().lonToString();
    // auto base = mMap->getProjection()->geoToProj(pos1);
    // QGV::GeoRect pos = mMap->getProjection()->projToGeo({ base, base + QPointF(size.x(), size.y()) });
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
