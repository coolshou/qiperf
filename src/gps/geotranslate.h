#ifndef GEOTRANSLATE_H
#define GEOTRANSLATE_H

#include <QtMath>
#include <QObject>
#include <QGeoCoordinate>

class GeoTranslate : public QObject
{
public:
    explicit GeoTranslate(QObject *parent = nullptr);

    static constexpr double pi = 3.14159265358979323846;
    static constexpr double a = 6378245.0;
    static constexpr double ee = 0.00669342162296594323;
	//WGS84坐標系 <=> 星坐標系(國測局坐標系、gcj02)
    Q_INVOKABLE static QGeoCoordinate wgs84ToGcj02(QGeoCoordinate wgLoc);
    Q_INVOKABLE static QGeoCoordinate gcj02ToWgs84(QGeoCoordinate gcLoc);
    Q_INVOKABLE static QGeoCoordinate wgs84ToGcj02(double lat,double lon);
    Q_INVOKABLE static QGeoCoordinate gcj02ToWgs84(double lat,double lon);
	// 火星坐標系(國測局坐標系、gcj02) <=> 百度坐標系(bd-09)
    Q_INVOKABLE static QGeoCoordinate gcj02ToBd09(QGeoCoordinate coordinate);
    Q_INVOKABLE static QGeoCoordinate bd09ToGcj02(QGeoCoordinate coordinate);
    Q_INVOKABLE static QGeoCoordinate gcj02ToBd09(double gg_lat, double gg_lon);
    Q_INVOKABLE static QGeoCoordinate bd09ToGcj02(double bd_lat,double bd_lon);

private:
    static double transformLat(double x,double y);
    static double transformLon(double x,double y);
    static bool outOfChina(double lat,double lon);
    static QGeoCoordinate transform(double lat,double lon);
    QGeoCoordinate bdEncrypt(QGeoCoordinate gcLoc);
    QGeoCoordinate bdDecrypt(QGeoCoordinate bdLoc);
};

#endif // GEOTRANSLATE_H
