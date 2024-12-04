#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QWidget>
#include <QPalette>
#include <QColor>

#include "tp.h"

TPMgr::TPMgr(QObject *parent)
    : QAbstractItemModel(parent)
{
//    item = invisibleRootItem();
    rootItem = new TP(("Root"), ("Root"), nullptr);
    rootItem->setDataType(TPMgrData::root);
    m_intervals.clear();

    QWidget widget;
    QPalette palette = widget.palette();
    m_disabledTextColor = palette.color(QPalette::Disabled, QPalette::Text);
    m_updater = new QTimer();
    // m_updater->setInterval(1000); //1 sec
    connect(m_updater, &QTimer::timeout, this, &TPMgr::onUpdater);
    startUpdater();
}
TPMgr::~TPMgr()
{
    //    delete rootItem;
}
QVariant TPMgr::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()){
        qDebug() << "data index.isValid: " << idx ;
        return QVariant();
    }
    if (role == Qt::TextAlignmentRole){
        if ((idx.column() == TP::cols::throughput)||
            (idx.column() == TP::cols::mintp)||
            (idx.column() == TP::cols::maxtp)||
            (idx.column() == TP::cols::lostrate)){
            // align text data to center
            return Qt::AlignCenter;
        }
    }
    TP *item = getItem(idx);
    if (role == Qt::ForegroundRole){
        // when item is disabled, grayout text
        if (! item->getEnabled()) {
            return m_disabledTextColor;
        }
    }

   // if ((role == Qt::DecorationRole) && (idx.column()==TP::cols::id)) {
   //     //show a custom icon!!
   //     qDebug() << "show DecorationRole folder";
   //     return iconProvider.icon(QFileIconProvider::Folder);
   // }
    if (role != Qt::DisplayRole) {
        //this will show text data!!
        //        qDebug() << "data not DisplayRole:" << index << Qt::endl;
        return QVariant();
    }
//    if (role == Qt::DisplayRole || role == Qt::EditRole) {
//        TP* item = getItem(index);
//        return item->data(role);
//    }
//    return QVariant();
    //    qDebug() << "data:" << index << " ,role:" << QString::number(role) << Qt::endl;


//    TP *item = static_cast<TP*>(index.internalPointer());
    if (item->getDataType()==TPMgrData::config){
        if (idx.column()== TP::cols::dir) {
            if (!item->getEnabled()){
                return QVariant("disable"+item->data(idx.column()).toString());
            }
        }
        if ((idx.column() == TP::cols::throughput)||
            (idx.column() == TP::cols::mintp) ||
            (idx.column() == TP::cols::maxtp) ){
            if (item->data(idx.column()).toDouble()<0){
                //do not show -1 value
                return QVariant();
            }
        }
        //     (idx.column() == TP::cols::lostrate)){
        //     // TODO: this will keeps calc! not good
        //     //special case of throughput data (sum of all iperf  --parallel value)
        //     double tpvalue=0.0;
        //     double tpMinvalue=0.0;
        //     double tpMaxvalue=0.0;
        //     double tpLostrate=0.0;
        //     QList<TP *> tps=item->getChilds();
        //     foreach(TP *tp, tps){
        //         qDebug() << "TP: " << tp->getTxRxThroughput();
        //         tpvalue = tpvalue + tp->getTxRxThroughput().toDouble();
        //         tpMinvalue = tpMinvalue + tp->getMinThroughput().toDouble();
        //         tpMaxvalue = tpMaxvalue + tp->getMaxThroughput().toDouble();
        //         tpLostrate = tpLostrate + tp->getLostRate().toDouble();
        //         QCoreApplication::processEvents(QEventLoop::AllEvents);
        //     }
        //     if (idx.column()== TP::cols::throughput) {

        //         return QVariant(tpvalue);
        //     }
        //     if (idx.column()== TP::cols::mintp) {
        //         return QVariant(tpMinvalue);
        //     }
        //     if (idx.column()== TP::cols::maxtp) {
        //         return QVariant(tpMaxvalue);
        //     }
        //     if (idx.column()== TP::cols::lostrate) {
        //         return QVariant(tpLostrate);
        //     }
        // }
    }
//    if (idx.column()== TP::cols::lostrate) {
//        QModelIndex id = index(idx.row(), TP::cols::id, idx.parent());
//        TP *item = getItem(id);
//        qDebug() << item << " "<< item->row() << " getLostRate:" << item->getLostRate();
//        //TODO: special case of loserate date
//        return QVariant(item->getLostRate());
//    }

    return item->data(idx.column());
}

QVariant TPMgr::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
//    if (role != Qt::DisplayRole)
//        return QVariant();
    if (role == Qt::TextAlignmentRole){
        if (section != TP::cols::comment){
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
    TP *parentItem;
//    if (parent.column() > 0)
//        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TP*>(parent.internalPointer());

    return parentItem->childCount();
}

bool TPMgr::add(QString data)
{
    //json data
    int idx = rootItem->childCount();
    beginInsertRows(QModelIndex(), idx, idx);
    TP *tp = new TP(QString::number(idx), data, rootItem);
    tp->setDataType(TPMgrData::config);
    rootItem->appendChild(tp);
    endInsertRows();
//    qDebug() << "TPMgr::add: " << tp->getEnabled();

    return true;
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
    parent = rootItem;
    /*for(auto ch: parents){
        ix = index(ch->row(), 0, ix);
    }*/

    for(int i=0; i < parents.count(); i++){
        ix = index(parents[i]->row(), 0, ix);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    ix = index(ix.row(), 0, ix);
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
//    QList<TP *> tps;
    m_tps.clear();
    for(int i = 0; i<rootItem->childCount();i++){
        if (!showAll){
            if (!rootItem->child(i)->getEnabled()){
                continue;
            }
        }
        m_tps.append(rootItem->child(i));
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    return m_tps;
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
        for (int row = 0; row < rootItem->childCount(); ++row){
            TP *tp = rootItem->child(row);
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
        for (int row = 0; row < rootItem->childCount(); ++row){
            TP *tp = rootItem->child(row);
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
        foreach (const QJsonValue &value, jsonarr) {
            QJsonObject obj = value.toObject();
            QJsonDocument doc(obj);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            add(strJson);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        return true;
    }else{
        qDebug() << "TPMgr::loaddata wrong format:(" << error.errorString() <<")\n" << data;
        return false;
    }
}

void TPMgr::reset(){
    //reset all data to none
    beginResetModel();
    m_tps.clear();
    rootItem = new TP(("Root"), ("Root"));
    endResetModel();
    m_intervals.clear();
}

void TPMgr::clear(){
    // clean test record
    // TODO: when there is child the folding icon will not remove after clear!!
    if (rootItem->haveChilds()){
        foreach(auto tp, rootItem->getChilds()){
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
            return item;
        }else{
            qDebug() << "getItem: no item??";
        }
    }
    qDebug() << "getItem rootItem:" << rootItem;
    return rootItem;
}

TP *TPMgr::getRootItem() const
{
    return rootItem;
}

QModelIndex TPMgr::getRootItemIdx()
{
    QModelIndex idx = indexFromItem(rootItem);
    // qInfo() << "idx:" << idx << " rootItem:" << rootItem;
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
    TP *tp= getItem(midx);
    if (tp->getDirection().contains("Tx")){
        tp->setDirection("Rx");
    }else if (tp->getDirection().contains("Rx")){
        tp->setDirection("Tx");
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
    Q_UNUSED(unit)

    TP *tp = getItemByIdx(midx); //parent item
    if (tp==nullptr){
        // qDebug() << "addTPdata: no parent iperf pair?? (midx=" << midx << ")" << idx;
        return;
    }
    TP *c = getItemByIdx(midx+"_"+idx, tp); //iperf pair config item
    if (c==nullptr){
        c = new TP(midx+"_"+idx, "", tp);
        c->setThroughput(dir, value);
        c->setDirection(dir);
        c->setDataType(TPMgrData::TP);
        if (!pkt_lost.isEmpty()){
            if (!pkt_total.isEmpty()){
               // qDebug() << c << " new pkt_lost/pkt_total: " << pkt_lost << " / " << pkt_total;
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
        tp->appendChild(c); // add iperf pair config item to parent item
    }else{
        c->setThroughput(dir, value);
        if (!pkt_lost.isEmpty()){
            if (!pkt_total.isEmpty()){
               // qDebug() << c << " row:" << c->row() << " columnCount:" << c->columnCount() << " pkt_lost/pkt_total: " << pkt_lost << " / " << pkt_total;
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
//        qDebug() << "getItemByIdx:" << midx << " item:" << item;
        if (item->haveChilds()){
            if (item->childCount()>0){
                lst = item->getChilds();
            }else{
                return nullptr;
            }
        }else{
//            qDebug() << "item: " << item << " DO NOT have child";
            return nullptr;
        }
    }
    if(lst.count()>0){
        foreach (auto tp, lst){
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            if (tp->getID() == midx){
                return tp;
            }
        }
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
        foreach(auto ch, tp->getChilds()){
            mip.append(ch->getServer()+"_"+QString::number(ch->getPort()));
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
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
    foreach(auto tp, rootItem->getChilds()){
        if(m_ip == tp->getMgrServer() && (targetIP == tp->getServer())){
            port = tp->getPort();
            if (port>maxPort){
                maxPort = port;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return maxPort;
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
           int num = this->getMaxPort(o_server["manager"].toString(),
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
            this->add(strJson);
       }else{
           qDebug() << "Wrong format of clipboard data: " << data;
       }
    }
}

void TPMgr::startUpdater()
{
    if (!m_updater->isActive()){
        m_updater->start(1000); // 1 sec, TODO: why slow to show up the throughput/lost rate in cfg row??
    }
}

void TPMgr::stopUpdater()
{
    if (m_updater->isActive()){
        m_updater->stop();
    }
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
    //            qDebug() << "sInterval:" << sInterval << " idx:" << idx << " value:" << value << " packet: " << pkt_lost << " / " <<  pkt_total;
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

void TPMgr::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                            QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate)
{
    qDebug() << "TODO: TPMgr::onUpdateTPDatas, just show last value";
    // addTPdata(refrow, sInterval, idx, value, unit, dir, packetlosts, packettotals);
}

void TPMgr::onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                          QString value, QString unit, QString dir,
                          QString pkt_lost, QString pkt_total)
{
    qDebug() << "onUpdateTPAvg: " << idx << " time:" << sInterval << " : " << value
             << " lost/total:" << pkt_lost << "/" << pkt_total;
    addTPdata(midx, sInterval, idx, value, unit, dir, pkt_lost, pkt_total);
}

void TPMgr::onUpdater()
{   //update total throughput/lost rate for each iperf test pair (-P >=1) result
    double g_tpvalue=-1.0;
    int g_lostvalue=-1;
    int g_totalvalue=-1;

    for (auto &cfg : rootItem->getChilds() ) {
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
    rootItem->setThroughput(QString::number(g_tpvalue));
    rootItem->setLostRate(QString::number(g_lostvalue), QString::number(g_totalvalue));
}
