#ifndef TPDATA_H
#define TPDATA_H

#include <QString>
#include <QHash>

struct TPDataLost {
    double value;
    double lostRate;
};

struct TPDataGroup {
    double intervalId;
    QHash<QString, TPDataLost> dataPoints; // QString refIdx,
};

#endif // TPDATA_H
