#include "pingitem.h"

PingItem::PingItem(QString id, QObject *parent)
    : QObject{parent}
{
    m_id = id;
    m_enabled = true;
    m_itemDatas={m_id, "", "", "", // id, source, target, response
                 "", "", "", // min response, max response, lost rate
                 "", }; // comment
}

PingItem *PingItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

PingItem *PingItem::parentItem()
{
    return m_parentItem;
}

QVariant PingItem::data(int column) const
{
    if (column < 0 || column >= m_itemDatas.size()){
        return QVariant();
    }
    return m_itemDatas.at(column);
}

int PingItem::row() const
{
    if (m_parentItem){
        if (m_parentItem->haveChilds()){
            return m_parentItem->m_childItems.indexOf(const_cast<PingItem*>(this));
        }
    }
    return 0;
}

bool PingItem::haveChilds()
{
    if (m_childItems.count()>0){
        return true;
    }else{
        return false;
    }
}

int PingItem::childCount() const
{
    return m_childItems.count();
}

int PingItem::columnCount() const
{
    return m_itemDatas.count();
}

bool PingItem::getEnabled()
{
    return m_enabled;
}
