#ifndef TPMGRDATA_H
#define TPMGRDATA_H

#include <QObject>

class TPMgrData: public QObject
{
    Q_GADGET
public:
    enum DataType{
        root=0,  // root item
        config=1,  // config item
        TP=2,     // throughput data item
        group=3  // group of all config item
    };
    Q_ENUM(DataType)
};


#endif // TPMGRDATA_H
