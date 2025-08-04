#ifndef DIRECTIONARROW_H
#define DIRECTIONARROW_H

#include <QGeoView/QGVDrawItem.h>
#include <QObject>
#include <QPen>

class DirectionArrow : public QGVDrawItem
{
    Q_OBJECT
public:
    explicit DirectionArrow(QGV::GeoPos origin, double azimuthDeg, double length,
                            QColor color, qreal linewidth=3,
                            double arrowLength = 10.0, double arrowAngleDeg = 30.0);
private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
private:
    QGV::GeoPos mStartPos;
    QGV::GeoPos mEndPos;
    QGV::GeoPos mLeftPos;
    QGV::GeoPos mRightPos;
    double mAzimuthDeg;
    double mLength;
    QColor mColor;
    qreal mLineWidth;
    QPen mPen;

    QPolygonF mMainLinePoints;
    QPolygonF mLeftWingPoints;
    QPolygonF mRightWingPoints;
};



#endif // DIRECTIONARROW_H
