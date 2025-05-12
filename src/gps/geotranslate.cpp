#include "geotranslate.h"

#include <QDir>
#include <QDebug>

GeoTranslate::GeoTranslate(QObject *parent)
    :QObject(parent)
{

}

QGeoCoordinate GeoTranslate::wgs84ToGcj02(QGeoCoordinate wgLoc)
{
    QGeoCoordinate mgLoc;
    if (outOfChina(wgLoc.latitude(), wgLoc.longitude()))
    {
        mgLoc = wgLoc;
        return mgLoc;
    }
    double dLat = transformLat(wgLoc.longitude() - 105.0, wgLoc.latitude() - 35.0);
    double dLon = transformLon(wgLoc.longitude() - 105.0, wgLoc.latitude() - 35.0);
    double radLat = wgLoc.latitude() / 180.0 * pi;
    double magic = sin(radLat);
    magic = 1 - ee * magic * magic;
    double sqrtMagic = sqrt(magic);
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);
    mgLoc.setLatitude(wgLoc.latitude() + dLat);
    mgLoc.setLongitude(wgLoc.longitude() + dLon);

    return mgLoc;
}

QGeoCoordinate GeoTranslate::gcj02ToWgs84(QGeoCoordinate gcLoc)
{
    //TODO gcj02ToWgs84
    Q_UNUSED(gcLoc)
    QGeoCoordinate wgLoc = gcLoc;
    // QGeoCoordinate currGcLoc, dLoc;
    // while (1) {
    //     currGcLoc = transformFromWGSToGCJ(wgLoc);
    //     dLoc.lat = gcLoc.lat - currGcLoc.lat;
    //     dLoc.lng = gcLoc.lng - currGcLoc.lng;
    //     if (fabs(dLoc.lat) < 1e-7 && fabs(dLoc.lng) < 1e-7) {  // 1e-7 ~ centimeter level accuracy
    //         // Result of experiment:
    //         //   Most of the time 2 iterations would be enough for an 1e-8 accuracy (milimeter level).
    //         //
    //         return wgLoc;
    //     }
    //     wgLoc.lat += dLoc.lat;
    //     wgLoc.lng += dLoc.lng;
    // }

    return wgLoc;
}

QGeoCoordinate GeoTranslate::wgs84ToGcj02(double lat, double lon)
{
    //TODO
    QGeoCoordinate wgLoc(lat,lon);
    return wgs84ToGcj02(wgLoc);
}

QGeoCoordinate GeoTranslate::gcj02ToWgs84(double lat, double lon)
{
    Q_UNUSED(lat)
    Q_UNUSED(lon)
    //TODO gcj02ToWgs84
    QGeoCoordinate gcj02Loc(lat,lon);
    return gcj02Loc;
}

QGeoCoordinate GeoTranslate::gcj02ToBd09(QGeoCoordinate coordinate)
{
    Q_UNUSED(coordinate)
    //TODO gcj02ToBd09
    QGeoCoordinate gcj02Loc=coordinate;
    return gcj02Loc;
}

QGeoCoordinate GeoTranslate::bd09ToGcj02(QGeoCoordinate coordinate)
{
    Q_UNUSED(coordinate)
    //TODO bd09ToGcj02
    QGeoCoordinate bd09Loc=coordinate;
    return bd09Loc;
}

QGeoCoordinate GeoTranslate::gcj02ToBd09(double gg_lat, double gg_lon)
{
    // Q_UNUSED(gg_lat)
    // Q_UNUSED(gg_lon)
    //TODO gcj02ToBd09
    QGeoCoordinate gcj02Loc(gg_lat,gg_lon);
    return gcj02Loc;
}

QGeoCoordinate GeoTranslate::bd09ToGcj02(double bd_lat, double bd_lon)
{
    // Q_UNUSED(bd_lat)
    // Q_UNUSED(bd_lon)
    //TODO bd09ToGcj02
    QGeoCoordinate bd09Loc(bd_lat,bd_lon);
    return bd09Loc;
}

double GeoTranslate::convertEllipsoidToMSL(double lat, double lon, double ellipsoidHeight)
{
    //https://www.maps.ie/coordinates.html
    //conver GPS ellipsoidHeight (Altitude) to elevation (height above mean sea level)
    try {
        // const GeographicLib::Geoid& geoid = GeographicLib::Geoid::EGM96(); // or EGM2008
        // Construct with the geoid model name (must be installed!)
#ifdef Q_OS_LINUX
        QString geodatapath="/opt/qiperf/bin/geoids";
#elif Q_OS_WINDOWS
        QString geodatapath=qApp->applicationDirPath() + QDir::separator() + "geoids";
#else
        qDebug() << "Not support platform";
        return ellipsoidHeight;
#endif
        GeographicLib::Geoid geoid("egm96-5", geodatapath.toStdString());  // or "egm2008-1"
        double geoidHeight = geoid(lat, lon);
        return ellipsoidHeight - geoidHeight;
    } catch (const std::exception& e) {
        qWarning() << "Geoid error:" << e.what();
        return ellipsoidHeight;
    }
}

double GeoTranslate::calcElevationAngle(double h1, double h2, double distanceMeters)
{
    //h1, h2 in meter
    double deltaH = h2 - h1;
    double angleRadians = qAtan(deltaH / distanceMeters);
    return qRadiansToDegrees(angleRadians);
}

double GeoTranslate::transformLat(double x, double y)
{
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(x > 0 ? x:-x);
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 *sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;

    return ret;
}

double GeoTranslate::transformLon(double x, double y)
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(x > 0 ? x:-x);
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;

    return ret;
}

bool GeoTranslate::outOfChina(double lat, double lon)
{
    if((lon < 72.004) || (lon > 137.8347)){
        return 1;
    }
    if((lat < 0.8293) || (lat > 55.8271)){
        return 1;
    }
    return 0;
}

QGeoCoordinate GeoTranslate::transform(double lat, double lon)
{
    QGeoCoordinate loc(lat, lon);
    return loc;
}

QGeoCoordinate GeoTranslate::bdEncrypt(QGeoCoordinate gcLoc)
{
    // Transform GCJ-02 to BD-09
    return QGeoCoordinate(gcLoc.latitude() + 0.006, gcLoc.longitude() + 0.0065);
}

QGeoCoordinate GeoTranslate::bdDecrypt(QGeoCoordinate bdLoc)
{
    // Transform BD-09 to GCJ-02
    return QGeoCoordinate(bdLoc.latitude() - 0.006, bdLoc.longitude() - 0.0065);
}
