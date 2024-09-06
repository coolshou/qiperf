#include "tp.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPixmap>
#include <QVariant>

TP::TP(QString id, QString data, TP *parent)
    :m_id(id), m_parentItem(parent)
{
    m_id=id;
    m_jsondata = "";
//    setEnabled();
    m_enabled = true;
    m_datatype = 0;
    m_lostpacket = 0;
    m_totalpacket = 0;
    clearThroughput();

    m_itemDatas={m_id, "", "", "", // id, server, dir ,client
                 "", "", "", //throughput, min tput, max tput
                 "", "", }; // lost rate, comment
    if (data!="" && data !="Root"){
        loadData(data);
    }
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
    if (position < 0 || position + count > m_childItems.size()){
        return false;
    }
    m_childItems.erase(m_childItems.begin()+position, m_childItems.begin()+count);

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
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = doc.object();
        m_enabled = jsonRoot["enabled"].toBool();
        QJsonObject o_client = jsonRoot["client"].toObject();
        m_version = o_client["version"].toInt();
        QString client = o_client["bind"].toString();
        m_mgrclient = o_client["manager"].toString();
        m_port = o_client["port"].toInt();
        m_duration = o_client["duration"].toInt();
        m_omit = o_client["omit"].toInt();
        m_delaytime = o_client["delaytime"].toInt();
    //    QString m_mclient = o_client["manager"].toString();
        QString direction = QVariant::fromValue(DirType::Tx).toString();
        if (o_client["bidir"].toBool()){
            direction=QVariant::fromValue(DirType::TR).toString();
        }
        if (o_client["reverse"].toBool()){
            direction=QVariant::fromValue(DirType::Rx).toString();
        }

        QJsonObject o_server = jsonRoot["server"].toObject();
        QString server = o_client["target"].toString();
        m_mgrserver = o_server["manager"].toString();

        //m_itemDatas.clear();// this will remove all data => m_itemDatas.length()=0
        m_itemDatas.replace(int(TP::cols::id) , m_id);
        m_itemDatas.replace(int(TP::cols::server), server);
        m_itemDatas.replace(int(TP::cols::dir), direction);
        m_itemDatas.replace(int(TP::cols::client), client);
        m_itemDatas.replace(int(TP::cols::throughput), "");
        m_itemDatas.replace(int(TP::cols::mintp), "");
        m_itemDatas.replace(int(TP::cols::maxtp), "");
        m_itemDatas.replace(int(TP::cols::lostrate), "");
        m_itemDatas.replace(int(TP::cols::comment), "");
    }else{
        qDebug() << "TP::loadData wrong format (" << error.errorString() << "\n" << data;
    }
    m_jsondata = data;
}

QString TP::getJsonData(){
    return m_jsondata;
}
void TP::resetData(){
    //reset (clear) test data
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
{   // return Iperf server bind ip address
    return m_itemDatas[int(TP::server)].toString();
}

void TP::setServer(QString addr)
{
    m_itemDatas[int(TP::server)] = addr;
//    m_server = addr;
}

QString TP::getServerArgs()
{
    QJsonParseError error;
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = fulldoc.object();

        QJsonObject o_server = jsonRoot["server"].toObject();
        o_server["server"]=true;

        QJsonDocument doc(o_server);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        return strJson;
    }else{
        qDebug() << "getServerArgs wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return "";
    }
}

QString TP::getBindKey(bool smode)
{
    if (smode){
        return m_itemDatas[int(TP::server)].toString() + "_" + QString::number(m_port);
    }else{
        return m_itemDatas[int(TP::client)].toString() + "-" +
                m_itemDatas[int(TP::server)].toString() + "_" + QString::number(m_port);;
    }
}

QString TP::getClient()
{   // return Iperf client bind ip address
    return m_itemDatas[int(TP::client)].toString();
}

void TP::setClient(QString addr)
{
    m_itemDatas[int(TP::client)] = addr;
//    m_client = addr;
}

QString TP::getClientArgs()
{
    QJsonParseError error;
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = fulldoc.object();

        QJsonObject o_client = jsonRoot["client"].toObject();
        o_client["server"]=false;
        QJsonDocument doc(o_client);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        return strJson;
    }else{
        qDebug() << "getClientArgs wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return "";
    }
}

QString TP::getDirection()
{
    return m_itemDatas[int(TP::dir)].toString();
}

QString TP::getMgrServer()
{
    return m_mgrserver;
}

void TP::setMgrServer(QString addr)
{
    m_mgrserver = addr;
}

QString TP::getMgrClient()
{
    return m_mgrclient;
}

void TP::setMgrClient(QString addr)
{
    m_mgrclient = addr;
}

void TP::swapServerClient(QString mgrServer, QString server, QString mgrClient, QString client)
{   //update server/client ip address in json
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = doc.object();
        QJsonObject o_server = jsonRoot["server"].toObject();
        o_server["manager"] = mgrServer;
        setMgrServer(mgrServer);
        o_server["bind"] = server;
        setServer(server);
        jsonRoot["server"] = o_server;
        QJsonObject o_client = jsonRoot["client"].toObject();
        o_client["manager"] = mgrClient;
        setMgrClient(mgrClient);
        o_client["bind"] = client;
        o_client["target"] = server;
        setClient(client);
        jsonRoot["client"] = o_client;
        doc.setObject(jsonRoot);
        m_jsondata =doc.toJson(QJsonDocument::Compact);
    }else{
        qDebug() << "swapServerClient wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
    }
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

int TP::getDelaytime()
{
    return m_delaytime;
}

int TP::setDirection(DirType direction)
{
    QString sdirection = QVariant::fromValue(direction).toString();
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
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
//        setData(TP::cols::dir, sdirection);

        return 0;
    }else{
        qDebug() << "setDirection wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return 1;
    }
}

int TP::setDirection(QString direction)
{
    setData(TP::cols::dir, direction);
    if (!m_jsondata.isEmpty()){
        if (direction.contains("Tx")){
            setDirection(TP::Tx);
        }else if (direction.contains("Rx")){
            setDirection(TP::Rx);
        }else if (direction.contains("TR")){
            setDirection(TP::TR);
        }else {
            setDirection(TP::RT);
        }
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
    if (m_itemDatas[int(TP::comment)].isValid()){
        if (m_itemDatas[int(TP::comment)].toString()!=""){
            m = m_itemDatas[int(TP::comment)].toString() + "\n" + comment;
        }else{
            m = comment;
        }
    }else{
        m = comment;
    }
    m_itemDatas[int(TP::comment)] = m;
}

void TP::setThroughput(QString value)
{
    if ((m_itemDatas[int(TP::mintp)].toDouble()<=0 && (value.toDouble()>0))||
            (value.toFloat() < m_itemDatas[int(TP::mintp)].toDouble())){
        m_itemDatas[int(TP::mintp)] = value;
    }
    if (m_itemDatas[int(TP::maxtp)]==""||
            (value.toFloat() > m_itemDatas[int(TP::maxtp)].toDouble())){
        m_itemDatas[int(TP::maxtp)] = value;
    }
    m_itemDatas[int(TP::throughput)] = value;
}

void TP::setThroughput(QString dir, QString value)
{
    if (dir.contains("Tx")){
        m_Tx = value.toDouble();
        // qDebug() << "setThroughput:m_Tx: " << m_Tx;
        if (m_minTx==0 || (m_minTx> value.toDouble())){
            m_minTx = value.toDouble();
        }
        if (m_maxTx < value.toDouble()){
            m_maxTx = value.toDouble();
        }
    }else{
        m_Rx = value.toDouble();
        // qDebug() << "setThroughput:m_Rx: " << m_Rx;
        if (m_minRx==0 || (m_minRx> value.toDouble())){
            m_minRx = value.toDouble();
        }
        if (m_maxRx < value.toDouble()){
            m_maxRx = value.toDouble();
        }
    }
    setThroughput(value);
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
    // qDebug() << "getTxRxThroughput: " << v;
    if (v>0){
       return QString::number(v);
   }else{
       return "";
   }
}

QString TP::getMinThroughput()
{
    double v = m_minTx+m_minRx;
    if (v>0){
        return QString::number(v);
    }else{
        return "";
    }
}

QString TP::getMaxThroughput()
{
    double v = m_maxTx+m_maxRx;
    if (v>0){
        return QString::number(v);
    }else{
        return "";
    }
}

void TP::clearThroughput()
{
    m_Tx = 0;
    m_Rx = 0;
    m_minTx = 0;
    m_maxTx = 0;
    m_minRx = 0;
    m_maxRx = 0;
}

int TP::getLostPackets()
{
    return m_lostpacket;
}

int TP::getTotalPackets()
{
    return m_totalpacket;
}

void TP::setLostRate(QString pkt_lost, QString pkt_total)
{
    if (pkt_lost.toInt()>=0){
        m_lostpacket = pkt_lost.toInt();
    }
    if (pkt_total.toInt()>0){
        m_totalpacket = pkt_total.toInt();
    }else{
        m_totalpacket = 0;
    }
    if (m_totalpacket>0){
        double lr = static_cast<double>(m_lostpacket)/m_totalpacket;

        QString s= QString::number(lr*100)+
                " ("+QString::number(m_lostpacket)+"/"+QString::number(m_totalpacket)+")";

        m_itemDatas[int(TP::lostrate)] = s;
    }
}

QString TP::getLostRate()
{
    if (m_totalpacket>0){
        double v = (static_cast<double>(m_lostpacket) / m_totalpacket)*100;
        if (v>0){
            return QString::number(v);
            // +"("+ QString::number(m_lostpacket) +"/"+ QString::number(m_totalpacket) +")";
        }else{
            qDebug() << "lost/total:" << QString::number(m_lostpacket) << " / " << QString::number(m_totalpacket);
            return QString();
        }
    }else{
//        qDebug() << "m_totalpacket:" << QString::number(m_totalpacket);
        return QString();
    }
}

void TP::setEnabled()
{
    m_enabled = true;
    updateJson("enabled", m_enabled);
}

void TP::setDisabled()
{
    m_enabled = false;
    updateJson("enabled", m_enabled);
}

bool TP::getEnabled()
{
    return m_enabled;
}
void TP::updateJson(QString key, QVariant value){
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = doc.object();
        if (value.canConvert<bool>()){
            jsonRoot[key] = value.toBool();
        }else if (value.canConvert<QString>()) {
            jsonRoot[key] = value.toString();
        }else if (value.canConvert<int>()) {
            jsonRoot[key] = value.toInt();
        }else {
            qDebug() << "not support type of value: " << value << " type: "<< value.typeName();
        }
        doc.setObject(jsonRoot);
    //    qDebug() << "updateJson:jsonRoot" << jsonRoot;
        m_jsondata =doc.toJson(QJsonDocument::Compact);
    //    qDebug() << "updateJson:m_jsondata:" << m_jsondata;
    }else{
        qDebug() << "updateJson wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
    }
}
