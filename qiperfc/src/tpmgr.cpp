#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QWidget>
#include <QPalette>
#include <QColor>

#include <QDebug>

#include "tp.h"
#include "../src/tpmgrdata.h"

TPMgr::TPMgr(bool showgroup, QObject *parent)
    : QAbstractItemModel(parent), m_showgroup(showgroup)
{
    rootItem=nullptr;
//    item = invisibleRootItem();
    reset();
    m_intervals.clear();
    groupItem = nullptr;
    QWidget widget;
    QPalette palette = widget.palette();
    m_disabledTextColor = palette.color(QPalette::Disabled, QPalette::Text);
    m_updater = new QTimer();
    connect(m_updater, &QTimer::timeout, this, &TPMgr::onUpdater);
    startUpdater();

    // setTestData();
}
TPMgr::~TPMgr()
{
    delete rootItem;
    delete groupItem;
    // qDeleteAll(m_tps);
    // m_tps.clear();
}
QVariant TPMgr::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole){
        if ((index.column() == TP::cols::dir)||
            (index.column() == TP::cols::throughput)||
            (index.column() == TP::cols::mintp)||
            (index.column() == TP::cols::maxtp)||
            (index.column() == TP::cols::lostrate)){
            // align text data to center
            return Qt::AlignCenter;
        }
    }

    TP *item = static_cast<TP*>(index.internalPointer());
    if (role == Qt::ForegroundRole){
        // when item is disabled, grayout text
        if (! item->getEnabled()) {
            return m_disabledTextColor;
        }
    }
/*
    if (tpitem->getDataType()==TPMgrData::group){
        qDebug() << "data: group:" << tpitem;
        return QVariant(tpitem->data(index.column()));
    }
*/
    if (item->getDataType()==TPMgrData::group){
        if ((index.column() == TP::cols::throughput)||
            (index.column() == TP::cols::mintp) ||
            (index.column() == TP::cols::maxtp) ){
            if (item->data(index.column()).toDouble()<0){
                //do not show -1 value
                return QVariant();
            }
        }
    }
    if (item->getDataType()==TPMgrData::config){
        if (index.column()== TP::cols::dir) {
            if (!item->getEnabled()){
                //when item disabled, let image grayout too.
                return QVariant("disable"+item->data(index.column()).toString());
             }
            //else { // the column dir will be empty!!
            //     return QVariant();
            // }
        }
        if ((index.column() == TP::cols::throughput)||
            (index.column() == TP::cols::mintp) ||
            (index.column() == TP::cols::maxtp) ){
            if (item->data(index.column()).toDouble()<0){
                //do not show -1 value
                return QVariant();
            }
        }
    }

    if (role != Qt::DisplayRole) {
        //this will show text data!!
        // qDebug() << "data not DisplayRole:" << idx ;
        return QVariant();
    }
    return item->data(index.column());
}

Qt::ItemFlags TPMgr::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}
QVariant TPMgr::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
//    if (role != Qt::DisplayRole)
//        return QVariant();
    if (role == Qt::TextAlignmentRole){
        if (section != TP::cols::comment){
            // all columns except "comment"
            return Qt::AlignCenter;
        }
    }
    // show header
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section)
        {
        case TP::cols::id:
            return QString("idx");
        case TP::cols::server:
            return QString("Server");
        case TP::cols::dir:
            return QString("Direction");
        case TP::cols::client:
            return QString("Client");
        case TP::cols::throughput:
            return QString("TPUT");
        case TP::cols::mintp:
            return QString("Min\nTPUT");
        case TP::cols::maxtp:
            return QString("Max\nTPUT");
        case TP::cols::lostrate:
            return QString("Lost Rate %\n(lost/total)");
        case TP::cols::comment:
            return QString("comment");
        default:
            return QVariant();
        }
    }

    return QVariant();
}

int TPMgr::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<TP*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}
QModelIndex TPMgr::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TP *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TP*>(parent.internalPointer());

    TP *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TPMgr::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();

    TP *childItem = static_cast<TP*>(idx.internalPointer());
    TP *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TPMgr::rowCount(const QModelIndex &parent) const
{
    // return correct child row count
    TP *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        // Root level: Number of parents
        return rootItem->childCount();//.size();
    }
    //TODO: groupItem?

    parentItem = static_cast<TP *>(parent.internalPointer());

    // qDebug() << "parent:" << parentItem;
    // qDebug() << "    childCount:" << parentItem->childCount();
    return parentItem->childCount();

//     int rowCount=0;
//     TP *parentItem;
// //    if (parent.column() > 0)
// //        return 0;

//     if (!parent.isValid()){
//         // if(m_showgroup){
//         //     qDebug() << "TPMgr::rowCount: parent: group" << groupItem;
//         //     parentItem = groupItem;
//         // }else{
//             // qDebug() << "TPMgr::rowCount: parent: root" << rootItem;
//             parentItem = rootItem;
//         // }
//     }else{
//         parentItem = static_cast<TP*>(parent.internalPointer());
//     }
//     rowCount = parentItem->childCount();
//     qDebug() << "parentItem:" << parentItem << " rowCount:" << QString::number(rowCount);
//     return rowCount;
}

TP *TPMgr::add(QString data, TPMgrData::DataType datatype,  TP *parent)
{   //add iperf config item
    TP *pitm = nullptr;
    if (parent){
        pitm = parent;
    }else{
        pitm = getRootItem();
        qInfo() << "add getRootItem " << pitm;
    }
    QModelIndex midx = indexFromItem(pitm);
    //data: json format data
    // qInfo() << "add data: " << data;
    //Get largest idx number!!
    int idx = getMaxIdx();
    // idx = idx + 1;
    // int idx = rootItem->childCount();
    beginInsertRows(midx, idx, idx);
    TP *tp = new TP(QString::number(idx), data, datatype, pitm);
    qDebug() <<"idx:" << idx << " tp:" << tp << " add pitm: " << pitm ;//<< " data:" << data;
    pitm->appendChild(tp);
    endInsertRows();

    return tp;
    // return true;
}
QModelIndex TPMgr::indexFromItem(TP *item){
    if(item == rootItem || item == nullptr)
        return QModelIndex();
    TP *parent = item->parentItem();

    QList<TP *> parents;

    while (parent && parent!=rootItem) {
        parents<<parent;
        parent = parent->parentItem();
    }
    QModelIndex ix;
    for(int i=0; i < parents.count(); i++){
        ix = index(parents[i]->row(), 0, ix);
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    ix = index(item->row(), 0, ix);
    return ix;
}

void TPMgr::del(QModelIndex idx)
{
    stopUpdater();
    int row = idx.row();
    if (!removeRows(row, 1 , getRootItemIdx())){
        qDebug() << "del fail("<< QString::number(row) << "): " << idx;
    }
    startUpdater();
}

int TPMgr::rootChildCount()
{
    // if(m_showgroup){
    //     return groupItem->childCount();
    // }else{
        return rootItem->childCount();
    // }
}

QList<TP *> TPMgr::getChilds(bool showAll)
{
    QList<TP *> tps;
    // m_tps.clear();
    TP *itm = getRootItem();
    // qDebug() << "root child:" << itm->childCount() << " cuilds: " << itm->getChilds()  ;

    for(int i = 0; i<itm->childCount();i++){
        TP *chitm = itm->child(i);
        if (!showAll){
            if (!chitm->getEnabled()){
                continue;
            }
        }
        // m_tps.append(itm->child(i));
        tps.append(chitm);
        if (chitm->haveChilds()){
            for(int j = 0; j<chitm->childCount();j++){
                // QCoreApplication::processEvents(QEventLoop::AllEvents);
                TP *ccitm = chitm->child(j);
                tps.append(ccitm);
            }
        }
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    // qDebug() << "getChilds tps:" << tps;
    return tps;
}

bool TPMgr::removeRows(int row, int count, const QModelIndex &parent)
{
    TP *parentItem = getItem(parent);
    bool success = true;
    qDebug() << "parentItem:" << parentItem << " type:" << parentItem->getDataType() ;
    beginRemoveRows(parent, row, row + count - 1);
    success = parentItem->removeChildren(row, count);
    endRemoveRows();

    return success;
}

QByteArray TPMgr::savedata()
{
    QJsonArray jsonarr;
    //save all data in json string
    if(rootChildCount() > 0){
        TP *itm = getRootItem();
        // if (m_showgroup){
        //     itm = groupItem;
        // }else{
        //     itm = rootItem;
        // }
        for (int row = 0; row < itm->childCount(); ++row){
            TP *tp = itm->child(row);
            QJsonDocument jsonDoc= QJsonDocument::fromJson(tp->saveData().toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            jsonarr.append(jsonObj);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }else {
        qDebug() << "TPMgr::savedata: No data to save";
    }
    QJsonDocument doc(jsonarr);
    return doc.toJson(QJsonDocument::Compact);
}

QStringList TPMgr::getPCs()
{
    QStringList ds;
    //get all config's PC info
    if(this->rootChildCount() > 0){
        TP *itm = getRootItem();
        // if (m_showgroup){
        //     itm = groupItem;
        // }else{
        //     itm = rootItem;
        // }
        for (int row = 0; row < itm->childCount(); ++row){
            TP *tp = itm->child(row);
//            qDebug() << "MgrServer: " << tp->getMgrServer();
//            qDebug() << "MgrClient: " << tp->getMgrClient();
            ds.append(tp->getMgrServer()+";"+tp->getMgrClient());
        }
    }else {
        qDebug() << "TPMgr::getPCs: No data to save" ;
    }
    return ds;
}

bool TPMgr::loaddata(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError){
        QJsonArray jsonarr = jsonDoc.array();
        // foreach (const QJsonValue &value, jsonarr) {
        for (const auto value: jsonarr){
            QJsonObject obj = value.toObject();
            QJsonDocument doc(obj);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            // qDebug() << "add: " << strJson;
            add(strJson);
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        return true;
    }else{
        qDebug() << "TPMgr::loaddata wrong format:(" << error.errorString() <<")\n" << data;
        return false;
    }
}

void TPMgr::reset(){
    //reset all data to none
    if (rootItem){
        delete rootItem;
    }
    rootItem = new TP(("Root"), ("Root"), TPMgrData::root); //
    qDebug() << "rootItem:" << rootItem;
    m_intervals.clear();
}

void TPMgr::clear(){
    // clean test record
    // TODO: when there is child the folding icon will not remove after clear!!
    TP *itm = getRootItem();
    if (itm->haveChilds()){
        foreach(auto tp, itm->getChilds()){
            if (tp->haveChilds()){
                tp->removeChildren(0, tp->childCount());
            }
            tp->clearThroughput();
            tp->resetData();
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        emit dataChanged(QModelIndex(),QModelIndex());
    }
    m_intervals.clear();
}

TP *TPMgr::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TP* item = static_cast<TP*>(index.internalPointer());
        if (item){
            // qDebug() << "TPMgr::getItem:" << item;
            return item;
        }else{
            qDebug() << "getItem: no item??";
        }
    }
    // TP *itm;// = getRootItem();
    // if (m_showgroup){
    //     itm = groupItem;
    // }else{
    //     itm = rootItem;
    // }
    // qDebug() << "getItem rootItem:" << itm;
    // return itm;
}

TP *TPMgr::getRootItem()
{
    if (m_showgroup){
        return getGroupItem();
    }else{
        return rootItem;
    }
}

TP *TPMgr::getGroupItem()
{
    if (groupItem){
        return groupItem;
    }else {
        return newGroupItem();
    }
}

QModelIndex TPMgr::getRootItemIdx()
{
    QModelIndex idx;
    idx = indexFromItem(getRootItem());
    qInfo() << "getRootItemIdx:" << idx;
    return idx;
}

void TPMgr::setItem(const QModelIndex &index, TP *item)
{
    if (index.isValid()) {
        qDebug() << "TODO: setItem:" << index << " item:" << item;
//        rootItem->appendChild();
        // TODO: setItem
    }
}

int TPMgr::swapDirection(QModelIndex midx)
{
    QString dir = TPDIRRx;
    TP *tp= getItem(midx);
    if (tp->getDirection().contains(TPDIRTx)){
        dir = TPDIRRx;
    }else if (tp->getDirection().contains(TPDIRRx)){
        dir = TPDIRTx;
    }
    tp->setDirection(dir);
    if (tp->haveChilds()){
        foreach(TP *t, tp->getChilds()){
            t->setDirection(dir);
        }
    }
    emit dataChanged(QModelIndex(),QModelIndex());
    return 0;
}

int TPMgr::swapIPDirection(QModelIndex midx)
{
    TP *tp= getItem(midx);
    QString server = tp->getServer();
    QString mgrServer = tp->getMgrServer();
    QString client = tp->getClient();
    QString mgrclient =tp->getMgrClient();
    tp->swapServerClient(mgrclient, client, mgrServer, server);

    emit dataChanged(QModelIndex(),QModelIndex());
    return 0;
}

void TPMgr::addComment(QString midx, QString comment)
{
    TP *tp = getItemByIdx(midx);
    if (tp==nullptr){
        qDebug() << "addComment: no parrent iperf pair?? (midx=" << midx << ")";
        return;
    }
//    qDebug() <<"TPMgr::addComment: " << tp << " midx:" << midx << " comment:" <<comment;
    tp->setComment(comment);
    emit dataChanged(QModelIndex(),QModelIndex());
}

void TPMgr::addTPdata(QString midx, QString sInterval, QString idx,
                      QString value, QString unit, QString dir,
                      QString pkt_lost, QString pkt_total)
{
    //add throughput item
    Q_UNUSED(sInterval)
    Q_UNUSED(unit) // TODO: check unit

    TP *tp = getItemByIdx(midx); //parent item
    if (tp==nullptr){
        return;
    }
    TP *c = getItemByIdx(midx+"_"+idx, tp); //iperf pair config item
    if (c==nullptr){
        //New
        c = new TP(midx+"_"+idx, "", TPMgrData::TP, tp);
        c->setThroughput(dir, value);
        c->setDirection(dir);
        if (!pkt_lost.isEmpty()){
            if (!pkt_total.isEmpty()){
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
        tp->appendChild(c); // add iperf pair config item to parent item
    }else{
        //update throughput value
        c->setThroughput(dir, value);
        if (!pkt_lost.isEmpty()){
            if (!pkt_total.isEmpty()){
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
    }
}

TP *TPMgr::getItemByIdx(QString midx, TP *item)
{
    QList<TP*> lst;
    if (item==nullptr){
        lst = getChilds();
    }else{
        if (item->haveChilds()){
            if (item->childCount()>0){
                lst = item->getChilds();
            }else{
                return nullptr;
            }
        }else{
            return nullptr;
        }
    }
    if(lst.count()>0){
        foreach (auto tp, lst){
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
            if (tp->getID() == midx){
                return tp;
            }
        }
    }else{
        qDebug() << "TPMgr::getItemByIdx: No TP list";
    }
    return nullptr;
}

QMap<QString, QStringList> TPMgr::getBindkeys()
{
    //TODO:  get all getBindkeys (managerIP_IP_Port)
    // each manager qiperfd's IP:Port can not be duplicate
    QMap<QString, QStringList> ds;
    QString mip;
    foreach (auto tp, this->getChilds()){
        mip = tp->getMgrServer();
        if(tp->haveChilds()){
            foreach(auto ch, tp->getChilds()){
                mip.append(ch->getServer()+"_"+QString::number(ch->getPort()));
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    //TODO: what is this??
    return ds;
}

bool TPMgr::isBindkeyExist(QString managerIP, QString bindkey, QModelIndex exc_idx)
{
    // QString mip;
    QString sbindkey;
    foreach (auto tp, this->getChilds()){
        if (exc_idx.isValid()){
            if (this->getItem(exc_idx) == tp){
                //skip same item
                continue;
            }
        }
//
        if(tp->getMgrServer().indexOf(managerIP)==0){
            sbindkey= tp->getServer()+"_"+QString::number(tp->getPort());
            if (sbindkey.indexOf(bindkey)==0){
                return true;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return false;
}

int TPMgr::getMaxPort(QString m_ip, QString targetIP)
{
    int maxPort=0;
    int port;
    TP *itm = getRootItem();
    if(itm->haveChilds()){
        foreach(auto tp, itm->getChilds()){
            if(m_ip == tp->getMgrServer() && (targetIP == tp->getServer())){
                port = tp->getPort();
                if (port>maxPort){
                    maxPort = port;
                }
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    return maxPort;
}

int TPMgr::getMaxIdx()
{
    TP *itm = getRootItem();
    int maxIdx=0;
    int idx = 0;
    foreach(auto tp, itm->getChilds()){
        idx = tp->getID().toInt();
        if (idx>maxIdx){
            maxIdx = idx;
        }else{
            maxIdx++;
        }
    }
    return maxIdx;
}

void TPMgr::onPaste(QString data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if(!doc.isNull())
    {
       QJsonObject jsonRoot = doc.object();
       if (jsonRoot.contains("client") &&jsonRoot.contains("server")){
           QJsonObject o_client = jsonRoot["client"].toObject();
           QJsonObject o_server = jsonRoot["server"].toObject();
           // Get largest iperf port number!!
           int num = getMaxPort(o_server["manager"].toString(),
                                         o_client["target"].toString());
            o_client["port"]=num+1;
            o_server["port"]=num+1;
            jsonRoot.remove("client");
            jsonRoot.remove("server");
            jsonRoot.insert("client", o_client);
            jsonRoot.insert("server", o_server);
            doc.setObject(jsonRoot);
            QString strJson(doc.toJson(QJsonDocument::Compact));
//          qDebug() << "strJson:\n" << strJson;
            add(strJson);
       }else{
           qDebug() << "Wrong format of clipboard data: " << data;
       }
    }
}

void TPMgr::startUpdater()
{
    if (!m_updater->isActive()){
        m_updater->start(500); // 1 sec, TODO: why slow to show up the throughput/lost rate in cfg row??
    }
}

void TPMgr::stopUpdater()
{
    if (m_updater->isActive()){
        m_updater->stop();
    }
}

void TPMgr::setTestData()
{
    // add test data to show tree
    // groupItem = new TP("Total", "Total", rootItem);
    TP *parentItm = getRootItem();
    // rootItem->appendChild(groupItem);
    QList<TP*> cfgs;

    QString cfg1="{\"Action\":\"IPERF_ADD\",\"client\":{\"bidir\":false,\"bind\":\"172.17.0.1\",\"bitrate\":0,\"buffer\":0,\"delaytime\":0,\"dscp\":-1,\"duration\":30,\"fmtreport\":\"m\",\"interval\":1,\"ipv6\":false,\"manager\":\"192.168.70.147\",\"mss\":0,\"omit\":2,\"parallel\":1,\"port\":5201,\"protocal\":\"TCP\",\"reverse\":false,\"target\":\"169.254.11.54\",\"tos\":-1,\"unit_bitrate\":\"K\",\"unit_buffer\":\"K\",\"unit_windowsize\":\"K\",\"version\":\"3\",\"windowsize\":256,\"zerocopy\":false},\"enabled\":true,\"server\":{\"bidir\":false,\"bind\":\"169.254.11.54\",\"delaytime\":0,\"fmtreport\":\"m\",\"interval\":1,\"manager\":\"192.168.70.11\",\"parallel\":1,\"port\":5201,\"protocal\":\"TCP\",\"reverse\":false,\"version\":\"3\"}}";
    QString cfg2="{\"Action\":\"IPERF_ADD\",\"client\":{\"bidir\":false,\"bind\":\"192.168.0.22\",\"bitrate\":0,\"buffer\":0,\"delaytime\":0,\"dscp\":-1,\"duration\":30,\"fmtreport\":\"m\",\"interval\":1,\"ipv6\":false,\"manager\":\"192.168.70.147\",\"mss\":0,\"omit\":2,\"parallel\":1,\"port\":5201,\"protocal\":\"TCP\",\"reverse\":false,\"target\":\"192.168.0.100\",\"tos\":-1,\"unit_bitrate\":\"K\",\"unit_buffer\":\"K\",\"unit_windowsize\":\"K\",\"version\":\"3\",\"windowsize\":256,\"zerocopy\":false},\"enabled\":true,\"server\":{\"bidir\":false,\"bind\":\"169.254.11.54\",\"delaytime\":0,\"fmtreport\":\"m\",\"interval\":1,\"manager\":\"192.168.70.11\",\"parallel\":1,\"port\":5201,\"protocal\":\"TCP\",\"reverse\":false,\"version\":\"3\"}}";

    TP *tpcfg = add(cfg1, TPMgrData::config, parentItm);
    cfgs << tpcfg;
    TP *tpcfg2 = add(cfg2, TPMgrData::config, parentItm);
    cfgs << tpcfg2;
    // TP *cfg = new TP("cfg1", "", TPMgrData::config, groupItem);
    // groupItem->appendChild(cfg);
    // qDebug() << "cfg:" << cfg;
    // cfgs << cfg;
    // TP *cfg2 = new TP("cfg2", "", TPMgrData::config, groupItem);
    // groupItem->appendChild(cfg2);
    // qDebug() << "cfg2:" << cfg2;
    // cfgs << cfg2;
    // foreach(TP *c, cfgs){
    //     qDebug() << "c:" << c;
    //     for (int i = 0; i < 3; ++i) {
    //         TP *child = new TP("f:"+QString::number(i), QString::number(i), TPMgrData::TP,  c);
    //         c->appendChild(child);
    //         for (int j = 0; j < 2; ++j) {
    //             TP *gchild = new TP("s:"+QString::number(j), QString::number(j), TPMgrData::TP, child);
    //             child->appendChild(gchild);

    //         }
    //     }
    // }

}

TP *TPMgr::newGroupItem()
{
    QModelIndex midx = indexFromItem(rootItem);
    qDebug() << " root idx: " << midx;
    beginInsertRows(midx, 0, 0);
    groupItem = new TP("0", "Total", TPMgrData::group, rootItem);
    rootItem->appendChild(groupItem);
    qDebug() << "newGroupItem: groupItem:" << groupItem << " root:" << rootItem;
    qDebug() << "newGroupItem: groupItem idx: " << indexFromItem(groupItem);
    endInsertRows();
    return groupItem;
}

void TPMgr::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    double fInterval = sInterval.toDouble();
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(datas.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        QJsonArray jArr = doc.array();//.object();
        //TODO: this only calc same reporter's value, in --bidir it will have two repoter!!
        QString dir=nullptr;
        QString idx;

        double sum=0;
        quint64 sum_lost=0;
        quint64 sum_total=0;
        double lost_rate=0;
        bool isAvg=false;
//        foreach (QJsonObject jObj, jArr){
        for (QJsonArray::const_iterator it=jArr.constBegin(); it!=jArr.constEnd(); ++it) {
            QJsonObject jObj= it->toObject();
            idx = jObj.value("idx").toString();
            isAvg = jObj.value("AVG").toBool();
            QString value="";
            if (!jObj.value("dir").isUndefined()){
                dir=jObj.value("dir").toString();
            }
            value = jObj.value("value").toString();
            // packet lost rate
            QString pkt_lost = jObj.value("packet_lost").toString();
            QString pkt_total = jObj.value("packet_total").toString();
            if ((pkt_total.toInt()>0) && (pkt_lost.toInt()>0)){
                lost_rate = (pkt_lost.toDouble()/pkt_total.toDouble())*100;
            }
    //        qDebug() << "pkt_lost/pkt_total: " << pkt_lost << " / " << pkt_total;
            sum = sum + value.toDouble();
            sum_lost = sum_lost + pkt_lost.toDouble();
            sum_total = sum_total + pkt_total.toDouble();
            if (fInterval >= m_intervals.value(idx, 0.0)){
               // qDebug() << "addTPdata fInterval:" << fInterval << " idx:" << idx << " value:" << value << " packet: " << pkt_lost << " / " <<  pkt_total;
                addTPdata(refrow, sInterval, idx, value,
                    jObj.value("unit").toString(), dir, pkt_lost, pkt_total);
                m_intervals[idx] = fInterval;
            }
            if (!isAvg) {
                // chart data ( with out Average data)
    //            qDebug() << sInterval <<" lost_rate: " << lost_rate;
                emit IperfTPdata(sInterval, refrow + "_" + jObj.value("idx").toString(),
                        jObj.value("value").toString(), QString::number(lost_rate, 'f', 4));
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        // update  test pair config row's sum value
        TP *tp = getItemByIdx(refrow);
        if (!(tp==nullptr)){
            if (fInterval >= m_intervals.value(refrow, 0.0)){
                //do not update sum when current m_intervals is larger sInterval
    //            qDebug() << "refrow:"<< refrow <<" isAvg:" << isAvg << " sInterval:" << sInterval << " sum:" << sum << " sum lost:" << sum_lost << " sum total:" << sum_total;
                tp->setThroughput(dir ,QString::number(sum));
                tp->setLostRate(QString::number(sum_lost), QString::number(sum_total));
                m_intervals[refrow] = fInterval;
            }
        }
        // signal dataChanged when all throughput data update!!
        emit dataChanged(QModelIndex(),QModelIndex());
    }else {
        qDebug() << "TPMgr::onIperfTPdata wrong format:(" << error.errorString() << "\n" << datas;
    }

}

void TPMgr::onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                          QString value, QString unit, QString dir,
                          QString pkt_lost, QString pkt_total)
{
    qDebug() << "onUpdateTPAvg: " << idx << " time:" << sInterval << " : " << value
             << " lost/total:" << pkt_lost << "/" << pkt_total;
    addTPdata(midx, sInterval, idx, value, unit, dir, pkt_lost, pkt_total);
}

void TPMgr::setShowGroup(bool bShow)
{
    qDebug() << "TODO: setShowGroup: " << bShow;
    m_showgroup = bShow;
    //beginMoveRows()
    //endMoveRows()
    if (m_showgroup){
        // if (groupItem==nullptr){
        //     TP *groupItem = newGroupItem();
        // }
        qDebug() << "ShowGroup: rootItem->childCount():" << rootItem->childCount();
        // TODO: move exist test item to groupItem
        // qDebug() << "groupItem:" << groupItem;
        // if (!groupItem){
        // rootItem->appendChild(groupItem);
        // }

    }else{
        qDebug() << "TODO: HideGroup: groupItem and move all subitem to rootItem";


    }
}

QModelIndex TPMgr::setSelectItem(QString idx)
{
    TP *tp = getItemByIdx(idx);
    if (tp){
        QModelIndex midx = indexFromItem(tp);
        return midx;
    }else{
        qDebug() << "DID not get " << idx << " TP item";
        return QModelIndex();
    }
}

void TPMgr::onUpdater()
{   //update total throughput/lost rate for each iperf test pair (-P >=1) result
    double g_tpvalue=-1.0;
    int g_lostvalue=-1;
    int g_totalvalue=-1;
    TP *itm;
    // if (m_showgroup){
    //     itm = groupItem;
    // }else{
        itm = rootItem;
    // }
    if (itm->haveChilds()){
        for (auto &cfg : itm->getChilds() ) {
            double tpvalue=-1.0;
            int lostvalue=-1;
            int totalvalue=-1;
            for (auto &tp: cfg->getChilds()){
                tpvalue = tpvalue + tp->getThroughput().toDouble();
                lostvalue = lostvalue + tp->getLostPackets();
                totalvalue = totalvalue + tp->getTotalPackets();
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            cfg->setThroughput(QString::number(tpvalue));
            cfg->setLostRate(QString::number(lostvalue), QString::number(totalvalue));
            g_tpvalue = g_tpvalue + tpvalue;
            g_lostvalue = g_lostvalue + lostvalue;
            g_totalvalue = g_totalvalue + totalvalue;
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        itm->setThroughput(QString::number(g_tpvalue));
        itm->setLostRate(QString::number(g_lostvalue), QString::number(g_totalvalue));
    }
}
