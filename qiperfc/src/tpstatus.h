#ifndef TPSTATUS_H
#define TPSTATUS_H

#include <QObject>

class TPStatus: public QObject
{
    Q_GADGET
public:
    enum Status{
        init=0,  // init
        ready=1, // wsclient ready
        started=2,  // iperf started
        stoped=3,     // iperf stoped
        restarted=4 //re-started
    };
    // init=-1,  // init
    //     started=0,  // started
    //     stoped=1     // stoped
    Q_ENUM(Status)
};

#endif // TPSTATUS_H
