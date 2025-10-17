#ifndef BEAMITEM_H
#define BEAMITEM_H

#include <QGVDrawItem.h>
#include <QGeoCoordinate>
#include <QPainterPath>
#include <QtMath>

class BeamItem : public QGVDrawItem {
public:
    BeamItem(const QGV::GeoPos& origin, double azimuthDeg, double hpbwDeg,
             double rangeMeters, const QColor& color);

    void setAzimuth(double azimuthDeg);
    void setOrigin(const QGV::GeoPos& origin);

protected:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;

private:
    double beamGaussianGain(double az);
    double butterworthGain(double az, double centerAz, double cutoff, int order);
    QGV::GeoPos atDistanceAndAzimuth(const QGV::GeoPos& origin, double distanceMeters, double azimuthDeg);

    QGV::GeoPos m_origin;
    double m_azimuth;
    double m_hpbw;
    double m_range;
    QColor m_color;
    QPainterPath m_path;
};

#endif // BEAMITEM_H
