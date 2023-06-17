#ifndef ENDPOINTMGR_H
#define ENDPOINTMGR_H

#include <QObject>
#include <QList>
#include "../src/endpoint.h"

class EndPointMgr : public QObject
{
    Q_OBJECT
public:
    explicit EndPointMgr(QObject *parent = nullptr);
    bool add(QString id, QString data); //add endpoint
    bool isExist(QString id);
    int getTotalEndpoints();

private:
    EndPoint* getEndPoint(QString id);

signals:
private:
    QList<EndPoint*> m_endpoints; //QList of endpoints
};

#endif // ENDPOINTMGR_H
