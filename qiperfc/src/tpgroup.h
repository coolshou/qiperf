#ifndef TPGROUP_H
#define TPGROUP_H

#include <QObject>

class TPGroup: public QObject
{
    Q_GADGET
public:
    enum GroupMode{
        Detail=0,    // detail mode
        Pair=0,      // Pair mode (each iperf test pair)
        Total=1,     // Total mode
        Direction=2, // Directiob mode (Tx & Rx)
        Comment=3    // custom mode (group by test pair's comment)
    };
    Q_ENUM(GroupMode)
};


#endif // TPGROUP_H
