#ifndef TPMGRDATA_H
#define TPMGRDATA_H

#include <QObject>

#define TPDIRTx "Tx"
#define TPDIRRx "Rx"
#define TPDIRTR "TR"
#define TPDIRNO ""

class TPMgrData: public QObject
{
    Q_GADGET
public:
    enum DataType{
        root=0,  // root item
        config=1,  // config item
        TP=2,     // throughput data item
        group=3,  // total group of all config item
        direction=4, // direction group
        comment=5       // comment group
    };
    Q_ENUM(DataType)
};


#endif // TPMGRDATA_H
