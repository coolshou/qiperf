#include "pingmgr.h"

#include <QWidget>
#include <QPalette>

#include <QDebug>

PingMgr::PingMgr(QObject *parent)
    : QAbstractItemModel{parent}
{
    QWidget widget;
    QPalette palette = widget.palette();
    m_disabledTextColor = palette.color(QPalette::Disabled, QPalette::Text);
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

QVariant PingMgr::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()){
        return QVariant();
    }
    PingItem *item = getItem(idx);
    if (role == Qt::ForegroundRole){
        // when item is disabled, grayout text
        if (! item->getEnabled()) {
            return m_disabledTextColor;
        }
    }
    if (role != Qt::DisplayRole) {
        //this will show text data!!
        return QVariant();
    }
    return item->data(idx.column());
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

PingItem *PingMgr::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        PingItem* item = static_cast<PingItem*>(index.internalPointer());
        if (item){
            return item;
        }else{
            qDebug() << "getItem: no item??";
        }
    }
    return rootItem;
}
