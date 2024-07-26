#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>


#include "tp.h"

TPMgr::TPMgr(QObject *parent)
    : QAbstractItemModel(parent)
{
//    item = invisibleRootItem();
    rootItem = new TP(("Root"), ("Root"), nullptr);
    rootItem->setDataType(TPMgrData::root);

}
TPMgr::~TPMgr()
{
    //    delete rootItem;
}
QVariant TPMgr::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        qDebug() << "data index.isValid: " << index ;
        return QVariant();
    }

//    if ((role == Qt::DecorationRole) && (index.column()==TP::cols::id)) {
//        //show a custom icon!!
//        qDebug() << "show DecorationRole folder";
//        return iconProvider.icon(QFileIconProvider::Folder);
//    }
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

    TP *item = static_cast<TP*>(index.internalPointer());
    if (item->getDataType()==TPMgrData::config){
        if (index.column()== TP::cols::throughput) {
            //special case of throughput data (sum of all iperf  --parallel value)
            return QVariant(item->getTxRxThroughput());
        }
        if (index.column()== TP::cols::mintp) {
            return QVariant(item->getMinThroughput());
        }
        if (index.column()== TP::cols::maxtp) {
            return QVariant(item->getMaxThroughput());
        }
    }
    if (index.column()== TP::cols::lostrate) {
        //TODO: special case of loserate date
        return QVariant(item->getLostRate());
    }
    return item->data(index.column());
}

QVariant TPMgr::headerData(int section, Qt::Orientation orientation,
                                 int role) const
{
//    if (role != Qt::DisplayRole)
//        return QVariant();
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
            return QString("Min \nTPUT");
        case TP::cols::maxtp:
            return QString("Max \nTPUT");
        case TP::cols::lostrate:
            return QString("Lost Rate (%)");
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

QModelIndex TPMgr::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TP *childItem = static_cast<TP*>(index.internalPointer());
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

int TPMgr::rootChildCount()
{
    return rootItem->childCount();
}

QList<TP *> TPMgr::getChilds()
{
//    QList<TP *> tps;
    m_tps.clear();
    for(int i = 0; i<rootItem->childCount();i++){
        m_tps.append(rootItem->child(i));
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }

    return m_tps;
}

bool TPMgr::removeRows(int position, int rows, const QModelIndex &parent)
{
    TP *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

QByteArray TPMgr::savedata()
{
    QJsonArray jsonarr;
    //save all data in json string
    if(this->rootChildCount() > 0){
        for (int row = 0; row < rootItem->childCount(); ++row){
            TP *tp = rootItem->child(row);
            QJsonDocument jsonDoc= QJsonDocument::fromJson(tp->saveData().toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            jsonarr.append(jsonObj);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }else {
        qDebug() << "TPMgr::savedata: No data to save" << Qt::endl;
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
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    QJsonArray jsonarr = jsonDoc.array();
    foreach (const QJsonValue &value, jsonarr) {
        QJsonObject obj = value.toObject();
        QJsonDocument doc(obj);
        QString strJson(doc.toJson(QJsonDocument::Compact));
        add(strJson);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return true;
}

void TPMgr::reset(){
    //reset all data to none
    beginResetModel();
    m_tps.clear();
    rootItem = new TP(("Root"), ("Root"));
    endResetModel();
}

void TPMgr::clear(){
    // clean test record
    // TODO: when there is child the folding icon will not remove after clear!!
    if (rootItem->haveChilds()){
        foreach(auto tp, rootItem->getChilds()){
            tp->removeChildren(0, tp->childCount());
            tp->setThroughput("Tx", 0);
            tp->setThroughput("Rx", 0);
            tp->resetData();
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        emit dataChanged(QModelIndex(),QModelIndex());
    }
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
    return rootItem;
}

TP *TPMgr::getRootItem() const
{
    return rootItem;
}

QModelIndex TPMgr::getRootItemIdx()
{
    QModelIndex idx = indexFromItem(rootItem);
    qDebug() << "idx:" << idx << " rootItem:" << rootItem;
    return idx;
}

void TPMgr::setItem(const QModelIndex &index, TP *item)
{
    if (index.isValid()) {
        qDebug() << "setItem:" << index << " item:" << item;
//        rootItem->appendChild();
        // TODO:
    }
}

int TPMgr::swapDirection(QModelIndex midx)
{
    TP *tp= getItem(midx);
    if (tp->getDirection().contains("Tx")){
        tp->setDirection(TP::DirType::Rx);
    }else if (tp->getDirection().contains("Rx")){
        tp->setDirection(TP::DirType::Tx);
    }
//    qDebug() << "after: " << tp->data(TP::cols::dir) << " midx: " <<midx;
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

void TPMgr::addTPdata(QString midx, QString sInterval, QString idx, QString value, QString unit, QString dir)
{
    Q_UNUSED(sInterval)
    Q_UNUSED(unit)

    TP *tp = getItemByIdx(midx); //parent item
    if (tp==nullptr){
        qDebug() << "addTPdata: no parent iperf pair?? (midx=" << midx << ")" << idx;
        return;
    }
    TP *c = getItemByIdx(midx+"_"+idx, tp); //iperf pair config item
    if (c==nullptr){
        c = new TP(midx+"_"+idx, "", tp);
        c->setThroughput(value);
        c->setDirection(dir);
        tp->appendChild(c); // add iperf pair config item to parent item
//        c->setExpanded(true);
    }else{
        c->setThroughput(value);
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
    QString mip;
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
//            qDebug() << "strJson:\n" << strJson;
            this->add(strJson);
       }else{
           qDebug() << "Wrong format of clipboard data: " << data;
       }
    }
}

void TPMgr::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    QJsonDocument doc=QJsonDocument::fromJson(datas.toUtf8());
    QJsonArray jArr = doc.array();//.object();
    //TODO: this only calc same reporter's value, in --bidir it will have two repoter!!
    QString dir=nullptr;
    double sum=0;
    foreach (auto jObj, jArr){
        QString value="";
        if (!jObj["dir"].isUndefined()){
            dir=jObj["dir"].toString();
        }
        value = jObj["value"].toString();
        sum = sum + value.toDouble();
        addTPdata(refrow, sInterval, jObj["idx"].toString(), value,
                jObj["unit"].toString(), dir);
        // chart data
        emit IperfTPdata(sInterval, refrow + "_" + jObj["idx"].toString(), jObj["value"].toString());
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    // signal dataChanged when all throughput data update!!
    TP *tp = getItemByIdx(refrow);
    if (!(tp==nullptr)){
        tp->setThroughput(dir ,QString::number(sum));
    }

    emit dataChanged(QModelIndex(),QModelIndex());

}
