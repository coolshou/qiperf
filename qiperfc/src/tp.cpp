#include "tp.h"
#include <QJsonDocument>
#include <QPixmap>

TP::TP(QString id, QString data, TP *parent)
    :m_parentItem(parent), m_id(id)
{
    m_id=id;
    m_itemDatas << id;
    m_jsondata = data;
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
    if (column==TP::cols::dir){
        int dir = m_itemDatas.at(column).toInt();
        if (dir==TP::DirType::Tx){
            QPixmap img(":/images/Tx");
            return img;
        }else if (dir==TP::DirType::Rx){
            QPixmap img(":/images/Rx");
            return img;
        }else{
            QPixmap img(":/images/TR");
            return img;
        }
    } else{
        return m_itemDatas.at(column);
    }
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
    qDebug() << "loadData:" << m_parentItem << Qt::endl;
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonRoot = doc.object();

    QJsonObject o_client = jsonRoot["client"].toObject();
    m_client = o_client["bind"].toString();
//    QString m_mclient = o_client["manager"].toString();
    int dir=DirType::Tx;
    if (o_client["bidir"].toBool()){
        dir=DirType::TR;
    }
    if (o_client["reverse"].toBool()){
        dir=DirType::Rx;
    }

    QJsonObject o_server = jsonRoot["server"].toObject();
    m_server = o_client["target"].toString();
//    QString m_mserver = o_server["manager"].toString();
//    m_itemDatas.append(m_mserver+"("+m_server+")");
    m_itemDatas.append(m_server);
    m_itemDatas.append(dir);
//    m_itemDatas.append(m_mclient+"("+m_client+")");
    m_itemDatas.append(m_client);
    m_itemDatas.append(""); //throughput
    m_itemDatas.append(""); //comment


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
