#include "tp.h"
#include <QJsonDocument>
#include <QPixmap>

TP::TP(QString id, QString data, TP *parent)
    :m_id(id), m_parentItem(parent)
{
    m_id=id;
//    m_itemDatas << id;
    m_jsondata = "";
    if (data!="" && data !="Root"){
        qDebug() << "TP data: "  << data;
        this->loadData(data);
    }
    m_datatype = 0;
    m_Tx = 0;
    m_Rx = 0;
    m_lostpacket = 0;
    m_totalpacket = 0;
}

void TP::appendChild(TP *item)
{
    m_childItems.append(item);
}

void TP::clear(){
    //TODO: following will cause proble!!
//    qDeleteAll(m_childItems);
//    m_childItems.clear();
}

int TP::findChild(TP *child)
{
    foreach(auto itm, m_childItems ){
        if (itm==child){
            qDebug() << "findChild FOUND:" << child ;
            break;
        }
    }
    return 0;
}

TP *TP::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

QList<TP *> TP::getChilds()
{
    return m_childItems;

}

int TP::childCount() const
{
    return m_childItems.count();
}

bool TP::haveChilds()
{
    if (m_childItems.count()>0){
        return true;
    }else{
        return false;
    }

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

int TP::setData(int column, QVariant var)
{
    if (column < 0 || column >= m_itemDatas.size()){
        return -1;
    }
    m_itemDatas[column]=var;
    return 0;
}

TP *TP::parentItem()
{
        return m_parentItem;
}

bool TP::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
    return false;

//    qDeleteAll(m_childItems);
    m_childItems.clear();
//    for (int row = position; row < count; ++row){
//        TP *tp =m_childItems.takeAt(row);
//        delete tp;
//        tp=nullptr;
//    }

    return true;
}

int TP::row() const
{
    //TODO: after clear, the may cause problem
    if (m_parentItem){
        if (m_parentItem->haveChilds()){
            return m_parentItem->m_childItems.indexOf(const_cast<TP*>(this));
        }
    }
    return 0;
}

QString TP::getID()
{
    return m_id;
}

void TP::loadData(QString data)
{
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonRoot = doc.object();

    QJsonObject o_client = jsonRoot["client"].toObject();
    m_version = o_client["version"].toInt();
    m_client = o_client["bind"].toString();
    m_mgrclient = o_client["manager"].toString();
    m_port = o_client["port"].toInt();
    qDebug() << "loadData m_port: " << m_port;
    m_duration = o_client["duration"].toInt();
    m_omit = o_client["omit"].toInt();

//    QString m_mclient = o_client["manager"].toString();
    m_direction = QVariant::fromValue(DirType::Tx).toString();
    if (o_client["bidir"].toBool()){
        m_direction=QVariant::fromValue(DirType::TR).toString();
    }
    if (o_client["reverse"].toBool()){
        m_direction=QVariant::fromValue(DirType::Rx).toString();
    }

    QJsonObject o_server = jsonRoot["server"].toObject();
    m_server = o_client["target"].toString();
    m_mgrserver = o_server["manager"].toString();

    m_itemDatas.clear();
    m_itemDatas.insert(cols::id,  m_id);
    m_itemDatas.insert(cols::server, m_server);
    m_itemDatas.insert(cols::dir, m_direction);
    m_itemDatas.insert(cols::client, m_client);
    m_itemDatas.insert(cols::throughput, "");
    m_itemDatas.insert(cols::mintp, "");
    m_itemDatas.insert(cols::maxtp, "");
    m_itemDatas.insert(cols::lostrate, "");
    m_itemDatas.insert(cols::comment, "");

    m_jsondata = data;
}

QString TP::getJsonData(){
    return m_jsondata;
}
void TP::resetData(){
    //reset (clear) test data
//    clear();
    QString d = getJsonData();
    loadData(d);

}
QString TP::saveData()
{
    return m_jsondata;
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
    o_server["server"]=true;
    QJsonDocument doc(o_server);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString TP::getBindKey(bool smode)
{
    if (smode){
        return m_server+"_"+QString::number(m_port);
    }else{
        return m_client + "-" + m_server + "_" + QString::number(m_port);;
    }
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
    o_client["server"]=false;
    QJsonDocument doc(o_client);
    QString strJson(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

QString TP::getDirection()
{
    return m_direction;
}

QString TP::getMgrServer()
{
    return m_mgrserver;
}

QString TP::getMgrClient()
{
    return m_mgrclient;
}

QString TP::getThroughput()
{
    return data(TP::throughput).toString();
}

int TP::getWaitTime()
{
    //omit time + test duration
    return m_omit + m_duration;
}

int TP::setDirection(DirType direction)
{
    m_direction = QVariant::fromValue(direction).toString();

    QJsonDocument doc= QJsonDocument::fromJson(m_jsondata.toUtf8());
    QJsonObject jsonRoot = doc.object();
    QJsonObject o_client = jsonRoot["client"].toObject();
    if (direction == DirType::Tx){
        o_client["bidir"]=false;
        o_client["reverse"]=false;
    }else if (direction == DirType::Rx){
        o_client["bidir"]=false;
        o_client["reverse"]=true;
    }else if (direction == DirType::TR){
        o_client["bidir"]=true;
        o_client["reverse"]=false;
    }else {
        o_client["bidir"]=true;
        o_client["reverse"]=true;
    }
    jsonRoot["client"]=o_client;
    doc.setObject(jsonRoot);
    m_jsondata =doc.toJson(QJsonDocument::Compact);
    setData(TP::cols::dir, m_direction);

    return 0;
}

int TP::setDirection(QString direction)
{
    if (direction.contains("Tx")){
        setDirection(TP::Tx);
    }else if (direction.contains("Rx")){
        setDirection(TP::Rx);
    }else if (direction.contains("TR")){
        setDirection(TP::TR);
    }else {
        setDirection(TP::RT);
    }
    return 0;
}

int TP::getPort()
{
    return m_port;
}

void TP::setComment(QString comment)
{
    QString m;
    if (m_itemDatas[TP::comment].isValid()){
        if (m_itemDatas[TP::comment].toString()!=""){
            m = m_itemDatas[TP::comment].toString() + "\n" + comment;
        }else{
            m = comment;
        }
    }else{
        m = comment;
    }
    m_itemDatas[TP::comment] = m;
}

void TP::setThroughput(QString value)
{
    m_itemDatas[TP::throughput] = value;
}

void TP::setThroughput(QString dir, QString value)
{
    if (dir.contains("Tx")){
        m_Tx = value.toDouble();
    }else{
        m_Rx = value.toDouble();
    }
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

void TP::setDataType(int datatype)
{
    m_datatype = datatype;
}

int TP::getDataType()
{
    return m_datatype;
}

QString TP::getTxRxThroughput()
{
   double v = m_Tx+m_Rx;
   if (v>0){
       return QString::number(v);
   }else{
       return "";
   }
}

QString TP::getLostRate()
{
    if (m_totalpacket>0){
        double v = (m_lostpacket / m_totalpacket)*100;
        if (v>0){
            return QString::number(v) +
                    "("+ QString::number(m_lostpacket) +"/"+ QString::number(m_totalpacket) +")";
        }else{
            return QString();
        }
    }else{
        return QString();
    }
}
