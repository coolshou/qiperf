#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>

#include "tp.h"

TPMgr::TPMgr(QObject *parent)
    : QAbstractItemModel(parent)
{
    this->rootItem = new TP(("Root"), ("Root"));
}
TPMgr::~TPMgr()
{
    //    delete rootItem;
}
QVariant TPMgr::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        qDebug() << "data index.isValid:" << index << Qt::endl;
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
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
//    //    EndPoint *item = itemFromIndex(index);
//    QVariant t = item->data(index.column());
//    qDebug() << "data: " << t;
//    return t;
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
    return this->rootItem->childCount();
}

QList<TP *> TPMgr::getChilds()
{
//    QList<TP *> tps;
    m_tps.clear();
    for(int i = 0; i<this->rootItem->childCount();i++){
        m_tps.append(this->rootItem->child(i));
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
    this->rootItem = new TP(("Root"), ("Root"));
    endResetModel();
}

void TPMgr::clear(){
    // clean test record
    if (this->rootItem->haveChilds()){
        qDebug()<< "clear";
        this->rootItem->clear();
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

void TPMgr::addTPdata(QString midx, QString sInterval, QString idx, QString value, QString unit, QString dir)
{
    Q_UNUSED(sInterval)
    Q_UNUSED(unit)
    Q_UNUSED(dir)
    TP *tp = getItemByIdx(midx);
    if (tp==nullptr){
        qDebug() << "no parrent iperf pair??";
        return;
    }
    TP *c = getItemByIdx(midx+"_"+idx, tp);
    if (c==nullptr){
//        beginInsertRows(QModelIndex(),tp->childCount(),tp->childCount());
        c = new TP(midx+"_"+idx, "", tp);
        c->setThroughput(value);
//        c->setExpanded(true);
        tp->appendChild(c);
//        endInsertRows();
    }else{
        c->setThroughput(value);
    }
    emit dataChanged(QModelIndex(),QModelIndex());
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

void TPMgr::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    QJsonDocument doc=QJsonDocument::fromJson(datas.toUtf8());
    QJsonArray jArr = doc.array();//.object();
    double sum=0;
    foreach (auto jObj, jArr){
        QString dir=nullptr;
        if (!jObj["dir"].isUndefined()){
            dir=jObj["dir"].toString();
        }
        sum = sum + jObj["value"].toString().toDouble();
        this->addTPdata(refrow, sInterval, jObj["idx"].toString(),
                jObj["value"].toString(), jObj["unit"].toString(), dir);
        // chart data
        emit IperfTPdata(sInterval, refrow + "_" + jObj["idx"].toString(), jObj["value"].toString());
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    TP *tp = getItemByIdx(refrow);
    if (!(tp==nullptr)){
        tp->setThroughput(QString::number(sum));
        emit dataChanged(QModelIndex(),QModelIndex());
    }
}
