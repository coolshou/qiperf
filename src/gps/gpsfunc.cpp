
#include "gpsfunc.h"

#include <QtMath>

#include <QDebug>

double toRadians(double degree){
    return degree * M_PI / 180.0;
}

double calcBearing(double lat1, double lon1, double lat2, double lon2)
{
    // calc point1 (lat1, lon1) face point2 (lat2, lon2) degree
    lat1 = toRadians(lat1);
    lon1 = toRadians(lon1);
    lat2 = toRadians(lat2);
    lon2 = toRadians(lon2);

    double deltaLon = lon2 - lon1;
    double y = qSin(deltaLon) * qCos(lat2);
    double x = qCos(lat1) * qSin(lat2) - qSin(lat1) * qCos(lat2) * qCos(deltaLon);
    double bearing = qAtan2(y, x);

    bearing = qRadiansToDegrees(bearing); // Convert to degrees
    bearing = fmod((bearing + 360.0), 360.0); // Normalize to 0-360

    return bearing;
}

double haversine(double lat1, double lon1, double lat2, double lon2)
{
    /* calc point A to B distance (KM)
     * 點 A：經度 lon1，緯度 lat1
     * 點 B：經度 lon2，緯度 lat2
    */
    const double R = 6371.0; // 地球半徑 (公里)

    // 轉換成弧度
    double lat1Rad = qDegreesToRadians(lat1);
    double lon1Rad = qDegreesToRadians(lon1);
    double lat2Rad = qDegreesToRadians(lat2);
    double lon2Rad = qDegreesToRadians(lon2);

    double dLat = lat2Rad - lat1Rad;
    double dLon = lon2Rad - lon1Rad;

    double a = qSin(dLat / 2) * qSin(dLat / 2) +
               qCos(lat1Rad) * qCos(lat2Rad) *
                   qSin(dLon / 2) * qSin(dLon / 2);

    double c = 2 * qAtan2(qSqrt(a), qSqrt(1 - a));

    double distance = R * c;

    return distance; // 單位：公里
}

/**
     * Vincenty 逆問題實現 - 計算兩點間的距離和方位角
     *
     * @param lat1 第一點緯度（度）
     * @param lon1 第一點經度（度）
     * @param lat2 第二點緯度（度）
     * @param lon2 第二點經度（度）
     * @return VincentyResult 包含距離（米）和兩個方位角（度）
     */
VincentyResult vincentyInverse(double lat1, double lon1, double lat2, double lon2) {
    // WGS-84 橢球體參數
    const double a = 6378137.0;         // 赤道半徑（米）
    const double f = 1.0 / 298.257223563; // 扁率
    const double b = a * (1.0 - f);     // 極半徑

    // 將度轉換為弧度
    const double phi1 = qDegreesToRadians(lat1);
    const double lambda1 = qDegreesToRadians(lon1);
    const double phi2 = qDegreesToRadians(lat2);
    const double lambda2 = qDegreesToRadians(lon2);

    // 計算輔助參數
    const double L = lambda2 - lambda1;  // 經度差

    // 計算橢球體參數
    const double tanU1 = (1.0 - f) * qTan(phi1);
    const double cosU1 = 1.0 / qSqrt(1.0 + tanU1 * tanU1);
    const double sinU1 = tanU1 * cosU1;

    const double tanU2 = (1.0 - f) * qTan(phi2);
    const double cosU2 = 1.0 / qSqrt(1.0 + tanU2 * tanU2);
    const double sinU2 = tanU2 * cosU2;

    // 初始化迭代變量
    double lambda = L;
    double sinLambda = 0.0;
    double cosLambda = 0.0;
    double sigma = 0.0;
    double sinSigma = 0.0;
    double cosSigma = 0.0;
    double cos2SigmaM = 0.0;
    double sinAlpha = 0.0;
    double cosSqAlpha = 0.0;
    double C = 0.0;

    // 迭代直到收斂
    double lambdaP = 0.0;
    int iterations = 0;
    const int maxIterations = 100;
    const double epsilon = 1e-12;  // 收斂閾值

    do {
        sinLambda = qSin(lambda);
        cosLambda = qCos(lambda);

        // 計算正弦和餘弦
        const double temp = cosU1 * sinU2 - sinU1 * cosU2 * cosLambda;
        sinSigma = qSqrt(cosU2 * cosU2 * sinLambda * sinLambda + temp * temp);

        if (sinSigma == 0.0) {
            // 兩點相同
            return {0.0, 0.0, 0.0};
        }

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = qAtan2(sinSigma, cosSigma);

        sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
        cosSqAlpha = 1.0 - sinAlpha * sinAlpha;

        // 處理極點附近的情況
        if (cosSqAlpha == 0.0) {
            cos2SigmaM = 0.0;
        } else {
            cos2SigmaM = cosSigma - 2.0 * sinU1 * sinU2 / cosSqAlpha;
        }

        C = f / 16.0 * cosSqAlpha * (4.0 + f * (4.0 - 3.0 * cosSqAlpha));

        lambdaP = lambda;
        lambda = L + (1.0 - C) * f * sinAlpha *
                         (sigma + C * sinSigma * (cos2SigmaM + C * cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM)));

        iterations++;
    } while (qAbs(lambda - lambdaP) > epsilon && iterations < maxIterations);

    // 如果迭代未收斂
    if (iterations >= maxIterations) {
        qWarning() << "Vincenty formula failed to converge!";
        return {0.0, 0.0, 0.0};
    }

    // 計算橢球體上的距離
    const double uSq = cosSqAlpha * (a * a - b * b) / (b * b);
    const double A = 1.0 + uSq / 16384.0 * (4096.0 + uSq * (-768.0 + uSq * (320.0 - 175.0 * uSq)));
    const double B = uSq / 1024.0 * (256.0 + uSq * (-128.0 + uSq * (74.0 - 47.0 * uSq)));

    const double deltaSigma = B * sinSigma * (cos2SigmaM + B / 4.0 * (cosSigma * (-1.0 + 2.0 * cos2SigmaM * cos2SigmaM) -
                                                                      B / 6.0 * cos2SigmaM * (-3.0 + 4.0 * sinSigma * sinSigma) * (-3.0 + 4.0 * cos2SigmaM * cos2SigmaM)));
    // m 米
    const double distance = b * A * (sigma - deltaSigma);

    // 計算方位角
    double initialBearing = qAtan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda);
    initialBearing = qRadiansToDegrees(initialBearing);
    // 確保方位角在 0-360 度之間
    initialBearing = fmod(initialBearing + 360.0, 360.0);

    double finalBearing = qAtan2(cosU1 * sinLambda, -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda);
    finalBearing = qRadiansToDegrees(finalBearing);
    // 確保方位角在 0-360 度之間
    finalBearing = fmod(finalBearing + 180.0, 360.0);

    return {distance, initialBearing, finalBearing};
}

DMS degreeToDegreeMinSec(double degree)
{
    //convert degree to DDD° MM' SS.SSSSS"
    int d = static_cast<int>(degree);
    double minuteDecimal = std::abs(degree - d) * 60.0;
    int m = static_cast<int>(minuteDecimal);
    double secondDecimal = std::abs(minuteDecimal - m) * 60.0;
    DMS dms;
    dms.degrees = d;
    dms.minutes = m;
    dms.seconds = secondDecimal;
    return dms;
}

DM degreeToDegreeMin(double degree)
{
    //convert degree to DDD° MM.MMMMM'
    int d = static_cast<int>(degree);
    double minuteDecimal = std::abs(degree - d) * 60.0;
    DM dm;
    dm.degrees = d;
    dm.minutes = minuteDecimal;
    return dm;
}
