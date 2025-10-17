#include "beamitem.h"

#include <QPainter>

BeamItem::BeamItem(const QGV::GeoPos &origin, double azimuthDeg,
                   double hpbwDeg, double rangeMeters, const QColor &color)
    : m_origin(origin), m_azimuth(azimuthDeg),
    m_hpbw(hpbwDeg), m_range(rangeMeters), m_color(color)
{

}

void BeamItem::setAzimuth(double azimuthDeg) {
    m_azimuth = azimuthDeg;
    // this->requestUpdate();
}

void BeamItem::setOrigin(const QGV::GeoPos &origin) {
    m_origin = origin;
    // requestUpdate();
}

void BeamItem::onProjection(QGVMap *geoMap) {
    QGVDrawItem::onProjection(geoMap);
    //TODO: create multi path with different gain?
    m_path = QPainterPath();
    QPointF center = geoMap->getProjection()->geoToProj(m_origin);
    m_path.moveTo(center);

    double startAz = m_azimuth - m_hpbw / 2;
    double endAz   = m_azimuth + m_hpbw / 2;

    for (double az = startAz; az <= endAz; az += 1.0) {
        double gain = beamGaussianGain(az); // 自訂函式，根據azimuth回傳增益（例如高斯分佈）
        // double gain = butterworthGain(az, m_azimuth, m_hpbw / 2.0, 4); //butterworth  階數 4
        double effectiveRange = m_range * gain; // 根據增益縮放距離
        QGV::GeoPos edge = atDistanceAndAzimuth(m_origin, effectiveRange, az);
        QPointF edgePoint = geoMap->getProjection()->geoToProj(edge);
        m_path.lineTo(edgePoint);
    }

    m_path.closeSubpath();
}

QPainterPath BeamItem::projShape() const {
    return m_path;
}

void BeamItem::projPaint(QPainter *painter) {
    //TODO: for multi path use different Alpha as gain
    QColor translucentColor = m_color;
    translucentColor.setAlpha(60); // 0 = 完全透明, 255 = 不透明

    painter->setBrush(QBrush(translucentColor));
    painter->setPen(Qt::NoPen);
    painter->drawPath(m_path);
}

double BeamItem::beamGaussianGain(double az)
{
    double delta = az - m_azimuth;
    double sigma = m_hpbw / 2.0;
    return exp(-0.5 * pow(delta / sigma, 2)); // 高斯分佈
}

double BeamItem::butterworthGain(double az, double centerAz, double cutoff, int order)
{
    double delta = az - centerAz;
    double norm = delta / cutoff;
    double denom = 1.0 + pow(norm, 2 * order);
    return 1.0 / sqrt(denom);
}

QGV::GeoPos BeamItem::atDistanceAndAzimuth(const QGV::GeoPos &origin, double distanceMeters, double azimuthDeg)
{
    constexpr double EarthRadius = 6371000.0;  // meters
    double lat1 = qDegreesToRadians(origin.latitude());
    double lon1 = qDegreesToRadians(origin.longitude());
    double bearing = qDegreesToRadians(azimuthDeg);

    double angularDistance = distanceMeters / EarthRadius;

    double lat2 = qAsin(qSin(lat1) * qCos(angularDistance) +
                        qCos(lat1) * qSin(angularDistance) * qCos(bearing));

    double lon2 = lon1 + qAtan2(qSin(bearing) * qSin(angularDistance) * qCos(lat1),
                                qCos(angularDistance) - qSin(lat1) * qSin(lat2));

    return QGV::GeoPos(qRadiansToDegrees(lat2), qRadiansToDegrees(lon2));
}
