#ifndef ENDPOINTACT_H
#define ENDPOINTACT_H

#include <QObject>

class EndPointAct : public QObject
{
    Q_OBJECT
public:
    explicit EndPointAct(QObject *parent = nullptr);
    enum Act{
        Init=0,
        Add=1,
        Update,
        Disable,
        Del
    };
    Q_ENUM(Act)
signals:

};

#endif // ENDPOINTACT_H
