#include "endpoint.h"
#include <QJsonDocument>


EndPoint::EndPoint(QString id, QString data, EndPoint *parent)
    :m_parentItem(parent), m_id(id)
{
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
//    QList<EndPoint *> parents;
    qDebug() << "loadData:" << m_parentItem << Qt::endl;
    //TODO: parser data
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonRoot = doc.object();
    m_type = static_cast<EndPointType::Type>(jsonRoot["Type"].toInt());
    EndPointType *ept = new EndPointType();
    QString sType = ept->getTypeString(m_type);
    m_Manager = jsonRoot["Manager"].toString();
//    bool update = jsonRoot["update"].toBool();
    OS_name = jsonRoot["OS"].toString();
    OS_version = jsonRoot["OSVer"].toString();
    m_itemDatas << m_id << m_Manager << sType << OS_name << OS_version;
    //    parents.last()->appendChild(new EndPoint(m_id, data, parents.last()));
    //TODO: get address of each interface....
    if (!jsonRoot["Net"].isNull()){
        oNet = jsonRoot["Net"].toObject();
        qDebug() << "TODO: oNet:" << oNet << Qt::endl;
    }

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
