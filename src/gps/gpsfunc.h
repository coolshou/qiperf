#ifndef GPSFUNC_H
#define GPSFUNC_H

// 結構體用於返回距離和方位角
struct VincentyResult {
    double distance;    // 距離（米）
    double initialBearing;  // 初始方位角（度）
    double finalBearing;    // 終點方位角（度）
};

enum class Direction {
    North,
    South,
    East,
    West
};
struct DM {
    int degrees;    // 度
    double minutes; // 分
    // Direction direction;
};

struct DMS {
    int degrees;    // 度
    int minutes;    // 分
    double seconds; // 秒
    // Direction direction;
};

double toRadians(double degree);
double calcBearing(double lat1, double lon1, double lat2, double lon2);
double haversine(double lat1, double lon1, double lat2, double lon2);
VincentyResult vincentyInverse(double lat1, double lon1, double lat2, double lon2);
DMS degreeToDegreeMinSec(double degree);
DM  degreeToDegreeMin(double degree);

#endif
