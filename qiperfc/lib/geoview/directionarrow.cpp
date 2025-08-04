#include "directionarrow.h"

#include <QGeoCoordinate>
#include <QPainter>
#include <QRectF>

DirectionArrow::DirectionArrow(QGV::GeoPos origin, double azimuthDeg, double length,
                               QColor color, qreal linewidth,
                               double arrowLength, double arrowAngleDeg):
    mStartPos(origin), mAzimuthDeg(azimuthDeg), mLength(length),
    mColor(color), mLineWidth(linewidth)
{
    mPen = QPen(QBrush(mColor), mLineWidth,
                         Qt::PenStyle::SolidLine,
                         Qt::PenCapStyle::RoundCap,
                         Qt::PenJoinStyle::RoundJoin);
    mMainLinePoints.resize(2);
    mLeftWingPoints.resize(2);
    mRightWingPoints.resize(2);
    //calc end pos
    QGeoCoordinate sPos(mStartPos.latitude(), mStartPos.longitude());
    QGeoCoordinate ePos = sPos.atDistanceAndAzimuth(mLength, mAzimuthDeg);
    mEndPos.setLat(ePos.latitude());
    mEndPos.setLon(ePos.longitude());

    //main line
    // addPoint(origin);
    // addPoint(ePos);
    // arrow wing
    double backAzimuth = azimuthDeg + 180.0;

    QGeoCoordinate leftWing = ePos.atDistanceAndAzimuth(
        arrowLength, backAzimuth - arrowAngleDeg);
    mLeftPos.setLat(leftWing.latitude());
    mLeftPos.setLon(leftWing.longitude());

    QGeoCoordinate rightWing = ePos.atDistanceAndAzimuth(
        arrowLength, backAzimuth + arrowAngleDeg);
    mRightPos.setLat(rightWing.latitude());
    mRightPos.setLon(rightWing.longitude());

    // left wing
    // addPoint(ePos);
    // addPoint(leftWing);

    // right wing
    // addPoint(ePos);
    // addPoint(rightWing);
}

void DirectionArrow::onProjection(QGVMap *geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    //translate geo point to line point
    mMainLinePoints[0] = geoMap->getProjection()->geoToProj(mStartPos);
    QPointF endF = geoMap->getProjection()->geoToProj(mEndPos);
    mMainLinePoints[1] = endF;

    mLeftWingPoints[0] = endF;
    mLeftWingPoints[1] = geoMap->getProjection()->geoToProj(mLeftPos);

    mRightWingPoints[0] = endF;
    mRightWingPoints[1] = geoMap->getProjection()->geoToProj(mRightPos);
}

QPainterPath DirectionArrow::projShape() const
{
    QPainterPath path;
    path.addPolygon(mMainLinePoints);
    path.addPolygon(mLeftWingPoints);
    path.addPolygon(mRightWingPoints);
    return path;
}

void DirectionArrow::projPaint(QPainter *painter)
{
    painter->setPen(mPen);
    //main line
    painter->drawPolyline(mMainLinePoints);
    painter->drawPolyline(mLeftWingPoints);
    painter->drawPolyline(mRightWingPoints);

}

