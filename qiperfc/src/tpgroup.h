#ifndef TPGROUP_H
#define TPGROUP_H

#include <QObject>

class TPGroup: public QObject
{
    Q_GADGET
public:
    enum GroupMode{
        Detail=0,    // detail mode
        Total=1,     // Total mode
        Pair=2,      // Pair mode (each iperf test pair)
        Direction=3, // Directiob mode (Tx & Rx)
        Comment=4    // custom mode (group by test pair's comment)
    };
    Q_ENUM(GroupMode)
};


#endif // TPGROUP_H
