#include "tp.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QPixmap>
#include <QVariant>
#include <QList>


TP::TP(QString id, QString data,int datatype, TP *parent)
    :m_id(id), m_datatype(datatype), m_parentItem(parent)
{
    m_childItems = QList<TP *>();
    m_childItems.clear();
    m_jsondata = "";
    m_enabled = true;
    m_lostpacket = 0;
    m_totalpacket = 0;
    clearThroughput();

    m_itemDatas={m_id, "", "", "", // id, server, dir ,client
                 "", "", "", //throughput, min throughput, max throughput
                 "", ""}; // lost rate, comment
    // qInfo() << "create TP:" << id << " data:" << data << " parent:" << parent;
    // if (data!="" && data !="Root" && data !="Total"){
    if (data!="" && m_datatype == TPMgrData::config){
        loadData(data);
    }else{
        if (data!=""){
            // m_itemDatas[1] = data;
            m_itemDatas[0] = data;
        }
    }
}

TP::~TP()
{
    // qDeleteAll(m_childItems);
    m_childItems.clear();
}

void TP::appendChild(TP *item)
{
    m_childItems.append(item);
}

void TP::clear(){
    qDebug() << "//TODO: clear child item ";
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
    // if (m_childItems.length()){
        return m_childItems.count();
    // }else {
        // return 0
    // }
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
    if (m_parentItem){
        return m_parentItem;
    }else{
        //TODO: parentItem
        return nullptr;
    }
}

bool TP::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()){
        return false;
    }
    QList<TP *>::ConstIterator begin = m_childItems.begin()+position;
    QList<TP *>::ConstIterator end = m_childItems.begin()+position+count;
    // m_childItems.erase(begin, end);
    for (int i = 0; i < count; ++i) {
        m_childItems.removeAt(position);  // Always remove at 'start'
    }
    return true;
}

TP * TP::removeChild(TP *child)
{
    int idx = m_childItems.indexOf(child);
    if (idx>=0){
        return m_childItems.takeAt(idx);
    }else{
        return nullptr;
    }
}

void TP::removeChild(int row)
{
    if (row >= 0 && row < m_childItems.size()) {
        m_childItems.removeAt(row);
    }
}

TP *TP::takeAt(int row)
{
    return m_childItems.takeAt(row);
}

void TP::insertChild(int row, TP *child)
{
    if (row >= 0 && row <= m_childItems.size()) {
        m_childItems.insert(row, child);
    }
}

void TP::setParent(TP *parent)
{
    m_parentItem = parent;
}

int TP::row() const
{
    //TODO: after clear, the may cause problem
    // if (m_parentItem != nullptr){
    // following must have for switch from No total group => total group setting
    if (m_parentItem){
        if (m_parentItem->haveChilds()){
            // m_parentItem->m_childItems

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
        m_enabled = jsonRoot.value("enabled").toBool(true);
        //client
        QJsonObject o_client = jsonRoot.value("client").toObject();
        m_version = o_client.value("version").toInt();
        QString client = o_client.value("bind").toString();
        m_mgrclient = o_client.value("manager").toString();
        m_port = o_client.value("port").toInt();
        m_duration = o_client.value("duration").toInt();
        m_omit = o_client.value("omit").toInt();
        m_delaytime = o_client.value("delaytime").toInt();
        m_interval = o_client.value("interval").toInt();
        m_parallel = o_client.value("parallel").toInt();
        m_protocal = o_client.value("protocal").toString();
    //    QString m_mclient = o_client["manager"].toString();
        QString direction = TPDIRRx;
        if (o_client.value("bidir").toBool()){
            direction = TPDIRTR;
        }
        if (o_client.value("reverse").toBool()){
            direction= TPDIRTx;
        }
        // server
        QJsonObject o_server = jsonRoot.value("server").toObject();
        QString server = o_client.value("target").toString();
        m_mgrserver = o_server.value("manager").toString();

        //m_itemDatas.clear();// this will remove all data => m_itemDatas.length()=0
        m_itemDatas.replace(int(TP::id) , m_id);
        m_itemDatas.replace(int(TP::server), server);
        m_itemDatas.replace(int(TP::dir), direction);
        m_itemDatas.replace(int(TP::client), client);
        m_itemDatas.replace(int(TP::throughput), "");
        m_itemDatas.replace(int(TP::mintp), "");
        m_itemDatas.replace(int(TP::maxtp), "");
        m_itemDatas.replace(int(TP::lostrate), "");
        m_itemDatas.replace(int(TP::comment), "");
    }else{
        qDebug() << "TP::loadData wrong format (" << error.errorString() << "\n data:" << data;
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
{   // get iperf server command arguments
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

QVariantMap TP::getServerArgsMap()
{
    QJsonParseError error;
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = fulldoc.object();

        QJsonObject jobj = jsonRoot.value("server").toObject();
        jobj["server"]=true;
        QJsonDocument doc(jobj);
        return doc.toVariant().toMap();
    }else{
        qDebug() << "getServerArgsMap wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return QVariantMap();
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
{   // get iperf client command arguments
    QJsonParseError error;
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = fulldoc.object();

        QJsonObject o_client = jsonRoot.value("client").toObject();
        o_client.value("server")=false;
        QJsonDocument doc(o_client);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        return strJson;
    }else{
        qDebug() << "getClientArgs wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return "";
    }
}

QVariantMap TP::getClientArgsMap()
{
    QJsonParseError error;
    QJsonDocument fulldoc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = fulldoc.object();

        QJsonObject o_client = jsonRoot.value("client").toObject();
        o_client.value("server")=false;
        QJsonDocument doc(o_client);
        return doc.toVariant().toMap();
    }else{
        qDebug() << "getClientArgsMap wrong format m_jsondata(" << error.errorString() << ")\n" << m_jsondata;
        return QVariantMap();
    }
}

QString TP::getDirection()
{
    return m_itemDatas[int(TP::cols::dir)].toString();
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
    // QString sdirection = QVariant::fromValue(direction).toString();
    QJsonParseError error;
    QJsonDocument doc= QJsonDocument::fromJson(m_jsondata.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        QJsonObject jsonRoot = doc.object();
        QJsonObject o_client = jsonRoot["client"].toObject();
        if (direction == DirType::Tx){
            o_client["bidir"]=false;
            o_client["reverse"]=true;
        }else if (direction == DirType::Rx){
            o_client["bidir"]=false;
            o_client["reverse"]=false;
        }else if (direction == DirType::TR){
            o_client["bidir"]=true;
            o_client["reverse"]=false;
        }else {
            o_client["bidir"]=true;
            o_client["reverse"]=true;
        }
        jsonRoot["client"]=o_client;
        QJsonObject o_server = jsonRoot["server"].toObject();
        if (direction == DirType::Tx){
            o_server["bidir"]=false;
            o_server["reverse"]=true;
        }else if (direction == DirType::Rx){
            o_server["bidir"]=false;
            o_server["reverse"]=false;
        }else if (direction == DirType::TR){
            o_server["bidir"]=true;
            o_server["reverse"]=false;
        }else {
            o_server["bidir"]=true;
            o_server["reverse"]=true;
        }
        jsonRoot["server"]=o_server;
        doc.setObject(jsonRoot);
        m_jsondata =doc.toJson(QJsonDocument::Compact);
        // setData(TP::cols::dir, sdirection);
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
        if (direction.contains(TPDIRTx)){
            setDirection(TP::DirType::Tx);
        }else if (direction.contains(TPDIRRx)){
            setDirection(TP::DirType::Rx);
        }else if (direction.contains(TPDIRTR)){
            setDirection(TP::DirType::TR);
        }else {
            setDirection(TP::DirType::RT);
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
    if (value.isEmpty()){
        // qDebug() << "setThroughput isEmpty";
        m_itemDatas[int(TP::mintp)] = "";
        m_itemDatas[int(TP::maxtp)] = "";
        m_itemDatas[int(TP::throughput)] = "";
    }else{
        //min value
        if ((m_itemDatas[int(TP::mintp)].toDouble()<=0 && (value.toDouble()>0))||
            (value.toFloat() < m_itemDatas[int(TP::mintp)].toDouble())){
            m_itemDatas[int(TP::mintp)] = value;
        }
        //max value
        if (m_itemDatas[int(TP::maxtp)]==""||
            (value.toFloat() > m_itemDatas[int(TP::maxtp)].toDouble())){
            m_itemDatas[int(TP::maxtp)] = value;
        }
        m_itemDatas[int(TP::throughput)] = value;
    }
}

void TP::setThroughput(QString dir, QString value)
{
    if (dir.contains(TPDIRTx)){
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
    m_lastnoticetime = t.toString("yyyy.MM.dd.hh:mm:ss.zzz");
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
    m_lostpacket = 0;
    m_totalpacket = 0;
    if (m_itemDatas.length()>0){
        m_itemDatas.replace(int(TP::throughput), "");
        m_itemDatas.replace(int(TP::mintp), "");
        m_itemDatas.replace(int(TP::maxtp), "");
        m_itemDatas.replace(int(TP::lostrate), "");
        m_itemDatas.replace(int(TP::comment), "");
    }
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
        //lost rate % not show in scientific notation eq: 5.83509e-05 (3/5141307)
        // float , Keep 2 Decimal points
        QString s= QString::number(lr*100, 'f', 2)+
                " ("+QString::number(m_lostpacket)+"/"+QString::number(m_totalpacket)+")";
        //TODO: only show rate, move lost/total to tooltip?
        m_itemDatas[int(TP::lostrate)] = s;
    }else {
        m_itemDatas[int(TP::lostrate)] = "";
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
    // m_enabled = true;
    // updateJson("enabled", m_enabled);
    setEnabled(true);
}

void TP::setEnabled(bool enable)
{
    m_enabled = enable;
    updateJson("enabled", m_enabled);
    if (m_childItems.count()>0){
        foreach(TP *tp, m_childItems){
            tp->setEnabled(m_enabled);
        }
    }
}

void TP::setDisabled()
{
    // m_enabled = false;
    // updateJson("enabled", m_enabled);
    setEnabled(false);
}

bool TP::getEnabled()
{
    return m_enabled;
}

int TP::getInterval()
{
    return m_interval;
}

int TP::getParallel()
{
    return m_parallel;
}

QString TP::getProtocal()
{
    return m_protocal;
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
