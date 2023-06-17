#include "endpointmgr.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

EndPointMgr::EndPointMgr(QObject *parent)
    : QObject{parent}
{

}

bool EndPointMgr::add(QString id, QString data)
{
//    QDateTime t=QDateTime::currentDateTime();
//    QString timestemp= t.toString("yyyy.dd.MM.hh:mm:ss.zzz");

    //check id exist
    if (isExist(id)){
        qDebug() << "TODO EndPointMgr::add: Exist(" << id << ") " << Qt::endl;
        QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
        QJsonObject jsonObject = doc.object();
//    int tpy = jsonObject["Type"].toInt();
        bool update = jsonObject["update"].toBool();
        EndPoint* ep = getEndPoint(id);
        if (update){
            //TODO: do data update!
            ep->loadData(data);
            ep->updateTimeStemp();
        }
        return false;
    } else {
        qDebug() << "TODO EndPointMgr::add: (" << id << ") " << data << Qt::endl;
        EndPoint* ep = new EndPoint(id, data, this);
        ep->updateTimeStemp();
        m_endpoints.append(ep);
        return true;
    }


}

bool EndPointMgr::isExist(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return true;
        }
    }
    return false;
}

int EndPointMgr::getTotalEndpoints()
{
    return m_endpoints.length();
}

EndPoint* EndPointMgr::getEndPoint(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return ep;
        }
    }
    return nullptr;
}
