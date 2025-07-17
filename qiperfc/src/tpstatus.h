#ifndef TPSTATUS_H
#define TPSTATUS_H

#include <QObject>

class TPStatus: public QObject
{
    Q_GADGET
public:
    enum Status{
        init=0,  // init
        started=1,  // started
        stoped=2,     // stoped
        restarted=3 //re-started
    };
    // init=-1,  // init
    //     started=0,  // started
    //     stoped=1     // stoped
    Q_ENUM(Status)
};

#endif // TPSTATUS_H
