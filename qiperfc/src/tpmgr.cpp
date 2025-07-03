#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QWidget>
#include <QPalette>
#include <QColor>

#include <QDebug>

#include "comm.h"
#include "tp.h"
#include "../src/tpmgrdata.h"
#include "../src/myfunc.h"

TPMgr::TPMgr(bool showgroup, QTreeView *treeview, QString tpunit, QObject *parent)
    : QAbstractItemModel(parent), m_showgroup(showgroup), m_treeview(treeview),
    m_TPUint(tpunit)
{
    connect(this, &QAbstractItemModel::rowsInserted, this, &TPMgr::onRowsInserted);
    m_unit_bits << "Kbits/sec" << "Mbits/sec" << "Gbits/sec" << "Tbits/sec";
    m_unit_bytes << "KBytes/sec" << "MBytes/sec" << "GBytes/sec" << "TBytes/sec";
    rootItem=nullptr;
    groupItem = nullptr;
//    item = invisibleRootItem();
    reset();
    m_intervals.clear();
    QWidget widget;
    QPalette palette = widget.palette();
    // QFont font = widget.font();
    // font
    m_disabledTextColor = palette.color(QPalette::Disabled, QPalette::Text);
    m_updater = new QTimer();
    connect(m_updater, &QTimer::timeout, this, &TPMgr::onUpdater);
    startUpdater();

}
TPMgr::~TPMgr()
{
    delete rootItem;
    delete groupItem;
}
QVariant TPMgr::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole){
        if ((index.column() == TP::dir)||
            (index.column() == TP::throughput)||
            (index.column() == TP::mintp)||
            (index.column() == TP::maxtp)||
            (index.column() == TP::lostrate)){
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
    if (role == Qt::DisplayRole){
        if((item->getDataType()==TPMgrData::group)||
           (item->getDataType()==TPMgrData::config)){
            if (item->childCount()>0) {
                if (index.column() == TP::throughput){
                    // sum of subitem's value
                    float sum = 0;
                    QString s;
                    int i=0;
                    for (TP* child : item->getChilds()) {
                        if (child->getThroughput().isEmpty()){
                            i++;
                        }else{
                            sum += child->getThroughput().toFloat();
                        }
                    }
                    if (i==item->getChilds().count()){
                        item->setThroughput("");
                        return QVariant();
                    }else{
                        if (sum>0){
                            item->setThroughput(s.setNum(sum));
                            return sum;
                        }
                    }
                }
                if (index.column() == TP::lostrate){
                    //calc Lost Rate
                    int lostpkts=0;
                    int totalpkts=0;
                    int i=0;
                    for (TP* child : item->getChilds()) {
                        lostpkts += child->getLostPackets();
                        totalpkts += child->getTotalPackets();
                        i++;
                    }
                    if (totalpkts){
                        item->setLostRate(QString::number(lostpkts), QString::number(totalpkts));
                    }
                }
            }else{
                // return QVariant(); //this will return nothing => cell show as empty!!
            }
        }
    }
    if ((item->getDataType()==TPMgrData::config)||(item->getDataType()==TPMgrData::group)){
        if ((index.column() == TP::throughput)||
            (index.column() == TP::mintp) ||
            (index.column() == TP::maxtp) ){
            if (item->data(index.column()).toDouble()<=0){
                //do not show  value < 0
                return QVariant();
            }
        }
    }
    if (item->getDataType()==TPMgrData::config){
        if (index.column()== TP::dir) {
            if (!item->getEnabled()){
                //when item disabled, let image grayout too.
                return QVariant("disable"+item->data(index.column()).toString());
             }
            //else { // the column dir will be empty!!
            //     return QVariant();
            // }
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
        case TP::id:
            return QString("idx");
        case TP::server:
            return QString("Server");
        case TP::dir:
            return QString("Direction");
        case TP::client:
            return QString("Client");
        case TP::throughput:
            return QString("TPUT\n(%1)").arg(MyFunc::formatUnit(m_TPUint));
        case TP::mintp:
            return QString("Min\nTPUT");
        case TP::maxtp:
            return QString("Max\nTPUT");
        case TP::lostrate:
            return QString("Lost Rate %\n(lost/total)");
        case TP::comment:
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
    if (!parentItem){
        // qDebug() << "==== NO parentItem";
        return QModelIndex();
    }
    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TPMgr::rowCount(const QModelIndex &parent) const
{
    // return correct child row count
    TP *parentItem;
    if (parent.column() > 0){
        return 0;
    }

    if (!parent.isValid()) {
        // Root level: Number of parents
        return rootItem->childCount();//.size();
    }
    //TODO: groupItem?
    parentItem = static_cast<TP *>(parent.internalPointer());

    // qDebug() << "rowCount parent:" << parentItem << " childCount:" << QString::number(parentItem->childCount());
    return parentItem->childCount();

}

TP *TPMgr::add(QString data, TPMgrData::DataType datatype,  TP *parent)
{   //add iperf config item
    TP *pitm = nullptr;
    if (parent){
        pitm = parent;
    }else{
        pitm = getRootItem();
        // qInfo() << "No parent, add:getRootItem " << pitm;
        // qInfo() << "datatype:" << datatype;
        // qInfo() << "TPMgr::add data:" << data;
    }
    QModelIndex midx = indexFromItem(pitm);
    int idx = getMaxIdx();
    beginInsertRows(midx, idx, idx);
    TP *tp = new TP(QString::number(idx), data, datatype, pitm);
    // qDebug() <<"Max idx:" << idx << " tp:" << tp << " add:"<< datatype << " pitm: " << pitm ;//<< " data:" << data;
    pitm->appendChild(tp);
    endInsertRows();
    return tp;
}
QModelIndex TPMgr::indexFromItem(TP *item){
    if(item == rootItem || item == nullptr)
        return QModelIndex();
    TP *parent = item->parentItem();

    QList<TP *> parents;

    while (parent && parent!=rootItem) {
        parents<<parent;
        parent = parent->parentItem();
        if (!parent){
            break;
        }
    }
    QModelIndex ix;
    for(int i=0; i < parents.count(); i++){
        ix = index(parents[i]->row(), 0, ix);
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
    return rootItem->childCount();
}

QList<TP *> TPMgr::getChilds(bool showAll)
{
    QList<TP *> tps;
    // m_tps.clear();
    TP *itm = getRootItem();
    // tps.append(itm);
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
    if (!parentItem){
        parentItem = rootItem;
    }
    bool success = true;
    if (parentItem){
        if (parentItem->childCount() >= (row+count)){
            beginRemoveRows(parent, row, row + count - 1);
            success = parentItem->removeChildren(row, count);
            endRemoveRows();
        }
    }
    return success;
}

bool TPMgr::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceRow < 0 || sourceRow + count > rowCount(sourceParent) ||
        destinationChild < 0 || destinationChild > rowCount(destinationParent)){
        return false;
    }
    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    //actual data structure change
    TP *sourceitem = getItem(sourceParent);
    if (!sourceitem){
        sourceitem = rootItem;
    }
    TP *destitem = getItem(destinationParent);
    if (!destitem){
        destitem = rootItem;
    }
    for (int i = sourceRow; i < count; i++) {
        TP *m = sourceitem->takeAt(0); // after take, the idx will change
        m->setParent(destitem);
        destitem->insertChild(i, m);
    }
    endMoveRows();
    return true;
}

bool TPMgr::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationChild)
{
    TP *sourceParentItem;
    if (!sourceParent.isValid()){
        qDebug() << " sourceParent.isValid:" << sourceParent.isValid();
        sourceParentItem = rootItem;
    }else {
        sourceParentItem = static_cast<TP*>(sourceParent.internalPointer());
    }
    TP *destinationParentItem;
    if (!destinationParent.isValid()){
        destinationParentItem = rootItem;
    }else{
        destinationParentItem = static_cast<TP*>(destinationParent.internalPointer());
    }

    if (sourceRow < 0 || sourceRow > sourceParentItem->childCount() ||
        destinationChild < 0 || destinationChild > destinationParentItem->childCount()){
        return false;
    }
    qDebug() << "sourceParent:" << sourceParent << " sourceRow:" << QString::number(sourceRow);
    qDebug() << "destinationParent:" << destinationParent << " destinationChild:" << QString::number(destinationChild);
    beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationChild);
    // data.move(sourceRow, destinationChild);
    sourceParentItem->removeChild(sourceParentItem);
    // rootItem->removeChild(sourceParentItem);
    // TP *item = sourceParentItem->child(sourceRow);
    // sourceParentItem->removeChild(sourceRow);
    destinationParentItem->insertChild(destinationChild, sourceParentItem);
    endMoveRows();
    return true;
}

QByteArray TPMgr::savedata()
{
    QJsonArray jsonarr;
    //save all data in json string
    if(rootChildCount() > 0){
        TP *itm = getRootItem();
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
        for (int row = 0; row < itm->childCount(); ++row){
            TP *tp = itm->child(row);
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
    // if (rootItem){
    //     if (rootItem->childCount()>0){
    //         // rootItem->removeChildren(0,rootItem->childCount());
    //     }
    //     // delete rootItem;
    // }
    if (m_showgroup){
        if (groupItem){
            if (groupItem->childCount()>0){
                groupItem->removeChildren(0,groupItem->childCount());
            }
            // delete groupItem;//direct delete cause app crash??
        }
    }
    rootItem = new TP(("Root"), ("Root"), TPMgrData::root); //
    // QModelIndex midx = indexFromItem(rootItem);
    // qDebug() << "rootItem:" << rootItem << " midx:" << midx << " valid:" << midx.isValid();
    m_intervals.clear();
    if (m_showgroup){
       if (groupItem!=nullptr){
           rootItem->appendChild(groupItem);
        }
    }
}

void TPMgr::clear(){
    // clean test record
    // TODO: when there is child the folding icon will not remove after clear!!
    TP *itm = getRootItem(); // rootitem or groupitem
    // qDebug() << "clear:" << itm->getDataType();
    if (itm){
        if (itm->haveChilds()){
            itm->clearThroughput();
            // itm->resetData();
            foreach(auto tp, itm->getChilds()){
                if (tp->haveChilds()){
                    // foreach(auto p, tp->getChilds()){ // parallel
                    beginRemoveRows(indexFromItem(tp), 0 , tp->childCount()-1);
                    tp->removeChildren(0, tp->childCount());
                    // m_treeview->collapse(indexFromItem(tp));
                    endRemoveRows();
                    // }
                }
                tp->clearThroughput();
                // tp->resetData();
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            emit dataChanged(QModelIndex(),QModelIndex());
        }
    }else {
        qDebug() << "clear: NO root item by getRootItem()";
    }

    m_intervals.clear();
}

TP *TPMgr::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TP* item = static_cast<TP*>(index.internalPointer());
        if (item){
            return item;
        }else{
            qDebug() << "getItem: no item??";
        }
    }
    return nullptr;
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
        // qDebug() << "getGroupItem:" << groupItem;
        return groupItem;
    }else {
        // qDebug() << "newGroupItem";
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
        qDebug() << "Direction:" << item->getDirection();
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
    // Q_UNUSED(unit) // TODO: check unit
    if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) !=0){
        qDebug() << " Expect unit:" << m_TPUint << " TP unit:" << unit;
    }
    TP *tp = getItemByIdx(midx); //parent item
    if (tp==nullptr){
        return;
    }
    TP *c = getItemByIdx(midx+"_"+idx, tp); //iperf pair config item
    if (c==nullptr){
        //New
        beginInsertRows(indexFromItem(tp), 0, 0);
        c = new TP(midx+"_"+idx, "", TPMgrData::TP, tp);
        c->setThroughput(dir, value);
        c->setDirection(dir);
        if (!pkt_lost.isEmpty()){
            if (!pkt_total.isEmpty()){
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
        tp->appendChild(c); // add iperf pair config item to parent item
        endInsertRows();
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
            // qDebug() << "tp->getID():" << tp->getID();
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
                if (tp->getProtocal().contains("UDP", Qt::CaseInsensitive)){
                    if (tp->getParallel()>1){
                        port = port + (tp->getParallel() - 1);
                    }
                }
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
//         qDebug() << "strJson:\n" << strJson;
           add(strJson);
       }else{
           qDebug() << "Wrong format of clipboard data: " << data;
       }
    }
}

void TPMgr::startUpdater()
{
    if (!m_updater->isActive()){
        m_updater->start(500); // 0.5 sec, TODO: why slow to show up the throughput/lost rate in cfg row??
    }
}

void TPMgr::stopUpdater()
{
    if (m_updater->isActive()){
        m_updater->stop();
    }
}

TP *TPMgr::newGroupItem()
{   // create new Total/Group item under rootItem
    QModelIndex midx = indexFromItem(rootItem);
    qDebug() << " root idx: " << midx;
    beginInsertRows(midx, 0, 0);
    groupItem = new TP("0", GRAPH_TOTAL, TPMgrData::group, rootItem);
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
        // qDebug() << "[TPMgr::onIperfTPdata]refrow(" << refrow << ") sInterval:" << sInterval
        //          << " QJsonArray size:" << jArr.size();
        //TODO: this only calc same reporter's value, in --bidir it will have two repoter!!
        QString dir=nullptr;
        QString idx;
        QString unit="";
        QString ssum="";
        double sum=0;
        quint64 sum_lost=0;
        quint64 sum_total=0;
        double lost_rate=0;
        bool isAvg=false;
        QString slost_rate = "0";
//        foreach (QJsonObject jObj, jArr){
        for (QJsonArray::const_iterator it=jArr.constBegin(); it!=jArr.constEnd(); ++it) {
            QJsonObject jObj= it->toObject();
            idx = jObj.value("idx").toString();
            isAvg = jObj.value("AVG").toBool();
            if (!isAvg) {
                QString value="";
                if (!jObj.value("dir").isUndefined()){
                    dir=jObj.value("dir").toString();
                }
                value = jObj.value("value").toString();
                unit = jObj.value("unit").toString();
                if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) !=0){
                    qDebug() << "//TODO: base on unit, convert the value to correct value"
                             << " display unit:" << m_TPUint << " tp data unit:" << unit;
                }
                // packet lost rate
                QString pkt_lost = jObj.value("packet_lost").toString();
                QString pkt_total = jObj.value("packet_total").toString();

                if ((pkt_total.toInt()>0) && (pkt_lost.toInt()>0)){
                    lost_rate = (pkt_lost.toDouble()/pkt_total.toDouble())*100;
                    qDebug() << "TPMgr::onIperfTPdata: lost_rate:" << lost_rate;
                    slost_rate = QString::number(lost_rate, 'f', 4);
                }
        //        qDebug() << "pkt_lost/pkt_total: " << pkt_lost << " / " << pkt_total;
                sum = sum + value.toDouble();
                sum_lost = sum_lost + pkt_lost.toDouble();
                sum_total = sum_total + pkt_total.toDouble();

                if (fInterval >= m_intervals.value(idx, 0.0)){
                   // qDebug() << "addTPdata fInterval:" << fInterval << " idx:" << idx << " value:" << value << " packet: " << pkt_lost << " / " <<  pkt_total;
                    addTPdata(refrow, sInterval, idx, value, unit, dir,
                              pkt_lost, pkt_total);
                    m_intervals[idx] = fInterval;
                }

                // signal data to tpplot to add plot data on each -P
                emit IperfTPdata(sInterval, refrow + "_" + idx, value,
                                 slost_rate, dir);
            }else {
                // TODO: this part TP data seems strange??
                // qDebug() << "refrow:" << refrow << " idx:" << idx <<" Avg:" << value
                         // << " pkt_lost:" << pkt_lost << " pkt_total:" << pkt_total;
                // TODO : each iperf test pair
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        // signal data to tpplot for Group Total
        if (sum_total>0){
            slost_rate = QString::number((sum_lost/sum_total)*100, 'f', 4);
        }
        ssum = QString::number(sum, 'f', 3);
        // qDebug() << " Total graph:" << sInterval << " sum:" << ssum
        //          << " lost_rate:" << slost_rate << " dir:" << dir;
        emit IperfTPdata(sInterval, GRAPH_TOTAL, ssum, slost_rate, dir);
        // TODO: signal data to tpplot for group Direction
        // TODO: signal data to tpplot for group comment
        // update  test pair config row's sum value

        // iperf test pair
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
        //
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
    m_showgroup = bShow;
    QModelIndex sourceparentidx;
    QModelIndex targetparentidx;
    int count =0;
    if (m_showgroup){  // not Total to show Total
        // move root's child to group
        count = rootItem->childCount();
        if (count>0){
            sourceparentidx = indexFromItem(rootItem);
            if (groupItem==nullptr){
                newGroupItem();
            }else {
                rootItem->appendChild(groupItem);
            }
            targetparentidx = indexFromItem(groupItem);
            if (targetparentidx.isValid()){
                moveRows(sourceparentidx, 0 , count, targetparentidx, 0);
            }
        }
    }else{
        // move group's child to root
        if (groupItem){
            count = groupItem->childCount();
            if (count>0){
                sourceparentidx = indexFromItem(groupItem);
                targetparentidx = indexFromItem(rootItem);
                moveRows(sourceparentidx, 0 , count, targetparentidx, 0);
                //remove groupItem
                rootItem->takeAt(groupItem->row());
            }
        }
    }
}

void TPMgr::onRowsInserted(const QModelIndex &parent, int first, int last)
{   //when inserted, expand all
    for (; first <= last; ++first) {
        m_treeview->expand(this->index(first, 0, parent));
    }
}

void TPMgr::setTPUint(QString tpunit)
{
    if (m_unit_bits.contains(tpunit) || m_unit_bytes.contains(tpunit)){
        m_TPUint = tpunit;
    } else {
        qDebug() << "unknown unit:" << tpunit;
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
    double g_tpvalue=0.0;
    int g_lostvalue=0;
    int g_totalvalue=0;
    TP *itm;
    // if (m_showgroup){
    //     itm = groupItem;
    // }else{
        itm = rootItem;
    // }
    if (itm->haveChilds()){
        for (auto &cfg : itm->getChilds() ) {
            double tpvalue=0.0;
            int lostvalue=0;
            int totalvalue=0;
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
