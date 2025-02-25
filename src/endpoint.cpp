#include "endpoint.h"
#include <QJsonDocument>

#include "../src/endpointmgr.h"

EndPoint::EndPoint(QString id, QString data, EndPoint *parent)
    :m_parentItem(parent), m_id(id)
{
    m_enabled = true;
    // m_itemDatas(EndPointMgr::cols::version+1 ,QVariant(""));
    // m_itemDatas.reserve(EndPointMgr::cols::version+1);
    this->loadData(data);
}

EndPoint::~EndPoint()
{
//    qDeleteAll(m_childItems);
}

void EndPoint::appendChild(EndPoint *item)
{
    m_childItems.append(item);
}

void EndPoint::delChild(EndPoint *item)
{
    int midx = m_childItems.indexOf(item);
    qDebug() << "(TODO delChild) midx:" << QString::number(midx);

}

EndPoint *EndPoint::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int EndPoint::childCount() const
{
    return m_childItems.count();
}

int EndPoint::columnCount() const
{
    return m_itemDatas.count();
}

QVariant EndPoint::data(int column) const
{
    if (column < 0 || column >= m_itemDatas.size())
        return QVariant();
    return m_itemDatas.at(column);
}

EndPoint *EndPoint::parentItem()
{
    return m_parentItem;
}

int EndPoint::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<EndPoint*>(this));

    return 0;
}

QString EndPoint::getID()
{
    return m_id;
}

void EndPoint::loadData(QString data)
{
    m_jsondata = data;
//    QList<EndPoint *> parents;
    //TODO: parser data
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8(), &error);
    QJsonObject jsonRoot = doc.object();
//    if (error.error == QJsonParseError::NoError){
    {
        m_type = static_cast<EndPointType::Type>(jsonRoot.value("Type").toInt());
        EndPointType *ept = new EndPointType();
        QString sType = ept->getTypeString(m_type);
        m_Manager = jsonRoot.value("Manager").toString(); // manager interface
        m_HostName = jsonRoot.value("HostName").toString();
        m_enabled = true;
    //    bool update = jsonRoot.value("update").toBool();
        OS_name = jsonRoot.value("OS").toString();
        OS_version = jsonRoot.value("OSVer").toString();
        qiperfd_ver = jsonRoot.value("qiperfd").toString();
        build_ver = jsonRoot.value("buildver").toString();
        m_itemDatas.insert(EndPointMgr::cols::name,  m_id);
        m_itemDatas.insert(EndPointMgr::cols::ifname, m_Manager);
        m_itemDatas.insert(EndPointMgr::cols::hostname, m_HostName);
        m_itemDatas.insert(EndPointMgr::cols::type, sType);
        m_itemDatas.insert(EndPointMgr::cols::os, OS_name);
        m_itemDatas.insert(EndPointMgr::cols::osver, OS_version);
        m_itemDatas.insert(EndPointMgr::cols::status, "");
        m_itemDatas.insert(EndPointMgr::cols::version, qiperfd_ver);
        m_itemDatas.insert(EndPointMgr::cols::buildver, build_ver);
        if (jsonRoot.value("serial").isArray()){
            m_serials = jsonRoot.value("serial").toArray();
        }
        updateTimeStemp();

        //TODO: get address of each interface....
        if (!jsonRoot.value("Net").isNull()){
            oNet = jsonRoot.value("Net").toObject();
        }
    }/*else{
        qDebug() << "EndPoint::loadData wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
    }*/
}

QString EndPoint::getJsonData()
{
    return m_jsondata;
}

void EndPoint::updateTimeStemp()
{
    QDateTime t=QDateTime::currentDateTime();
    m_lastnoticetime = t.toString("yyyy.MM.dd.hh:mm:ss.zzz");
    m_itemDatas[EndPointMgr::cols::status] = m_lastnoticetime;
}

QString EndPoint::getLastNoticeTime()
{
    return m_lastnoticetime;
}

void EndPoint::setEnabled(bool enable)
{
    m_enabled = enable;
}

bool EndPoint::getEnabled()
{
    return m_enabled;
}

QStringList EndPoint::getSerials()
{
    QStringList stringList;
    for (const QJsonValue &value : m_serials) {
        stringList.append(value.toString());
    }
    return stringList;
}
