#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonObject>

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
        case TP::cols::tp:
            return QString("Avg TP");
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
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<TP*>(parent.internalPointer());

    return parentItem->childCount();
}

bool TPMgr::add(QString data)
{
    //json data
    qDebug() << "TPMgr::add" << data << Qt::endl;
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
    }
    ix = index(ix.row(), 0, ix);
    return ix;
}

//TP *TPMgr::itemFromIndex(const QModelIndex &index) const
//{
//    if (index.isValid())
//    {
//        TP *item = static_cast<TP*>(index.internalPointer());
//        return item;
//    }
//    return rootItem;
//}

TP *TPMgr::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TP* item = static_cast<TP*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}
