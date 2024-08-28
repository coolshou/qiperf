#include "pingmgr.h"

PingMgr::PingMgr(QObject *parent)
    : QAbstractItemModel{parent}
{

}

QModelIndex PingMgr::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    PingItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PingItem*>(parent.internalPointer());

    PingItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex PingMgr::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();
    PingItem *childItem = static_cast<PingItem*>(idx.internalPointer());
    PingItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int PingMgr::rowCount(const QModelIndex &parent) const
{
    PingItem *parentItem;
//    if (parent.column() > 0)
//        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<PingItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int PingMgr::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<PingItem*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant PingMgr::headerData(int section, Qt::Orientation orientation, int role) const
{
    // show header
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section)
        {
        case PingItem::cols::id:
            return QString("idx");
        case PingItem::cols::source:
            return QString("Source");
        case PingItem::cols::target:
            return QString("Target");
        case PingItem::cols::rtime:
            return QString("Response");
        case PingItem::cols::minrtime:
            return QString("Min Response");
        case PingItem::cols::maxrtime:
            return QString("Max Response");
        case PingItem::cols::lostrate:
            return QString("Lost Rate %\n(lost/total)");
        case PingItem::cols::comment:
            return QString("comment");
        default:
            return QVariant();
        }
    }

    return QVariant();
}
