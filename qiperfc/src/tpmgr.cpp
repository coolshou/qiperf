#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonObject>

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

    //    qDebug() << "data:" << index << " ,role:" << QString::number(role) << Qt::endl;
    TP *item = static_cast<TP*>(index.internalPointer());
    //    EndPoint *item = itemFromIndex(index);
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
        case 0:
            return QString("idx");
        case 1:
            return QString("Server");
        case 2:
            return QString("Direction");
        case 3:
            return QString("Client");
        case 4:
            return QString("Avg TP");
        case 5:
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
//    QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
//    QJsonObject jsonObject = doc.object();
//    qDebug() << "client:" << jsonObject["client"] << Qt::endl;
//    qDebug() << "server:" << jsonObject["server"] << Qt::endl;
//    indexof
//    QModelIndex midx = indexFromItem(rootItem);
    int idx = rootItem->childCount();
//    midx = this->index(0,0);
//    idx = rowCount(this->rootItem);
    TP *tp = new TP(QString::number(idx), data, rootItem);
    rootItem->appendChild(tp);

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
