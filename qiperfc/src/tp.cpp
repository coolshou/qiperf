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

//TP::~TP()
//{
////    qDeleteAll(m_childItems);
//}

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
    if (column < 0 || column >= m_itemDatas.size()){
        return QVariant();
    }
    return m_itemDatas.at(column);

}

TP *TP::parentItem()
{
        return m_parentItem;
}

bool TP::removeChildren(int position, int count)
{
        if (position < 0 || position + count > m_childItems.size())
        return false;

        for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

        return true;
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
//    qDebug() << "loadData:" << m_parentItem << Qt::endl;
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonRoot = doc.object();

    QJsonObject o_client = jsonRoot["client"].toObject();
    m_version = o_client["version"].toInt();
    m_client = o_client["bind"].toString();
    m_mgrclient = o_client["manager"].toString();
    m_port = o_client["port"].toInt();
//    QString m_mclient = o_client["manager"].toString();
//    int dir=DirType::Tx;
    m_direction = QVariant::fromValue(DirType::Tx).toString();
    if (o_client["bidir"].toBool()){
//        dir=DirType::TR;
        m_direction=QVariant::fromValue(DirType::TR).toString();
    }
    if (o_client["reverse"].toBool()){
//        dir=DirType::Rx;
        m_direction=QVariant::fromValue(DirType::Rx).toString();
    }

    QJsonObject o_server = jsonRoot["server"].toObject();
    m_server = o_client["target"].toString();
    m_mgrserver = o_server["manager"].toString();

//    QString m_mserver = o_server["manager"].toString();
//    m_itemDatas.append(m_mserver+"("+m_server+")");
    m_itemDatas.append(m_server);
    m_itemDatas.append(m_direction);
//    m_itemDatas.append(m_mclient+"("+m_client+")");
    m_itemDatas.append(m_client);
    m_itemDatas.append(""); //throughput
    m_itemDatas.append(""); //comment


}

int TP::getVersion()
{
    return m_version;
}

QString TP::getServer()
{
    return m_itemDatas[TP::server].toString();
}

QString TP::getServerArgs()
{
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8());
    QJsonObject jsonRoot = fulldoc.object();

    QJsonObject o_server = jsonRoot["server"].toObject();

    QJsonDocument doc(o_server);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString TP::getClient()
{
    return m_itemDatas[TP::client].toString();
}

QString TP::getClientArgs()
{
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8());
    QJsonObject jsonRoot = fulldoc.object();

    QJsonObject o_client = jsonRoot["client"].toObject();
    QJsonDocument doc(o_client);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;

/*
    QString args;

    args.append("-B");
    args.append(o_client["bind"].toString());
    args.append("-p");
    args.append(o_client["port"].toString());
    if (o_client["protocal"].toString()=="UDP"){
        args.append("-u");
    }
    if (o_client["protocal"].toString()=="SCTP"){
        args.append("--sctp");
    }
    args.append("-c");
    args.append(o_client["target"].toString());
    args.append("-t");
    args.append(o_client["duration"].toString());
    args.append("-O");
    args.append(o_client["omit"].toString());
    args.append("-P");
    args.append(o_client["parallel"].toString());
    if (o_client["bitrate"].toInt()>=0) {
        args.append("-b");
        if (o_client["bitrate"].toInt()>0) {
            args.append(o_client["bitrate"].toString()+o_client["unit_bitrate"].toString());
        }else if(o_client["bitrate"].toInt()==0) {
            args.append(o_client["bitrate"].toString());
        }
    }
    if (o_client["windowsize"].toInt()>0) {
        args.append("-w");
        args.append(o_client["windowsize"].toString()+o_client["unit_windowsize"].toString());
    }
    if (o_client["buffer"].toInt()>0) {
        args.append("-l");
        args.append(o_client["buffer"].toString()+o_client["unit_buffer"].toString());
    }
    if (o_client["dscp"].toInt()>=0) {
        args.append("--dscp");
        args.append(o_client["dscp"].toString());
    }
    if (o_client["tos"].toInt()>=0) {
        args.append("--tos");
        args.append(o_client["tos"].toString());
    }
    if (o_client["mss"].toInt()>0) {
        args.append("--set-mss");
        args.append(o_client["mss"].toString());
    }
    args.append("-i");
    args.append(o_client["interval"].toString());
    args.append("--format");
    args.append(o_client["fmtreport"].toString());

    if (o_client["reverse"].toInt()>0) {
        args.append("-R");
    }
    if (o_client["bidir"].toInt()>0) {
        args.append("--bidir");
    }
    return args;
*/
}

QString TP::getMgrServer()
{
    return m_mgrserver;
}

QString TP::getMgrClient()
{
    return m_mgrclient;
}

int TP::getPort()
{
    return m_port;
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
