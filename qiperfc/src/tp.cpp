#include "tp.h"
#include <QJsonDocument>


TP::TP(QString id, QString data, TP *parent)
    :m_parentItem(parent), m_id(id)
{
    this->loadData(data);
}

TP::~TP()
{
//    qDeleteAll(m_childItems);
}

void TP::appendChild(TP *item)
{
    m_childItems.append(item);
}

TP *TP::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int TP::childCount() const
{
    return m_childItems.count();
}

int TP::columnCount() const
{
    return m_itemDatas.count();
}

QVariant TP::data(int column) const
{
    if (column < 0 || column >= m_itemDatas.size())
        return QVariant();
    return m_itemDatas.at(column);
}

TP *TP::parentItem()
{
    return m_parentItem;
}

int TP::row() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TP*>(this));

    return 0;
}

QString TP::getID()
{
    return m_id;
}

void TP::loadData(QString data)
{
//    QList<EndPoint *> parents;
    qDebug() << "loadData:" << m_parentItem << Qt::endl;
    //TODO: parser data
//    if (m_parentItem==0x0) {
        QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
        QJsonObject jsonRoot = doc.object();
//        m_type = static_cast<EndPointType::Type>(jsonRoot["Type"].toInt());
//        EndPointType ept = EndPointType();
//        QString sType = ept.getTypeString(m_type);
        m_Manager = jsonRoot["Manager"].toString();
    //    bool update = jsonRoot["update"].toBool();
        OS_name = jsonRoot["OS"].toString();
        OS_version = jsonRoot["OSVer"].toString();
//        m_itemDatas << m_id << m_Manager << sType << OS_name << OS_version;
        //    parents.last()->appendChild(new EndPoint(m_id, data, parents.last()));
        //TODO: get address of each interface....
        if (!jsonRoot["Net"].isNull()){
            oNet = jsonRoot["Net"].toObject();
            qDebug() << "oNet:" << oNet << Qt::endl;
        }
        /*
        QJsonObject oNet = jsonRoot["Net"].toObject();
        QStringList oNets = oNet.keys();
        foreach(QString sNet, oNets){
            QJsonValue jv = oNet.value(sNet);
            qDebug() << "if:" << sNet << " = " << jv["HW"].toString() << Qt::endl;
            qDebug() << "\taddress: " <<  jv["address"].toString() << Qt::endl;
        }*/
//    }
}

void TP::updateTimeStemp()
{
    QDateTime t=QDateTime::currentDateTime();
    m_lastnoticetime = t.toString("yyyy.dd.MM.hh:mm:ss.zzz");
}

QString TP::getLastNoticeTime()
{
    return m_lastnoticetime;
}
