#include "endpoint.h"
#include <QJsonDocument>
#include <QJsonObject>

EndPoint::EndPoint(QString id, QString data, QObject *parent)
    : QObject{parent}
{
    m_id = id;
    loadData(data);
}

QString EndPoint::getID()
{
    return m_id;
}

void EndPoint::loadData(QString data)
{
    //TODO: parser data
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonRoot = doc.object();
    m_type = static_cast<EndPoint::Type>(jsonRoot["Type"].toInt());
    m_Manager = jsonRoot["Manager"].toString();
//    bool update = jsonRoot["update"].toBool();
    OS_name = jsonRoot["OS"].toString();
    OS_version = jsonRoot["OSVer"].toString();

}

void EndPoint::updateTimeStemp()
{
    QDateTime t=QDateTime::currentDateTime();
    m_lastnoticetime = t.toString("yyyy.dd.MM.hh:mm:ss.zzz");
}

QString EndPoint::getLastNoticeTime()
{
    return m_lastnoticetime;
}
