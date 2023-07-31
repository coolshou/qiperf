#include "endpointmgr.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>

EndPointMgr::EndPointMgr(QObject *parent)
    : QAbstractItemModel(parent)
{
    this->rootItem = new EndPoint(("Root"), ("Root"));
}

EndPointMgr::~EndPointMgr()
{
//    delete rootItem;
}

int EndPointMgr::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<EndPoint*>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}

QVariant EndPointMgr::data(const QModelIndex &index, int role) const
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
    EndPoint *item = static_cast<EndPoint*>(index.internalPointer());
//    EndPoint *item = itemFromIndex(index);
    return item->data(index.column());
}

Qt::ItemFlags EndPointMgr::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant EndPointMgr::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    // show header
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section)
        {
            case 0:
                return QString("Manager");
            case 1:
                return QString("Interface");
            case 2:
                return QString("Type");
            case 3:
                return QString("OS");
            case 4:
                return QString("OS Ver");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QModelIndex EndPointMgr::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    EndPoint *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<EndPoint*>(parent.internalPointer());

    EndPoint *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex EndPointMgr::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    EndPoint *childItem = static_cast<EndPoint*>(index.internalPointer());
    EndPoint *parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int EndPointMgr::rowCount(const QModelIndex &parent) const
{
    EndPoint *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<EndPoint*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex EndPointMgr::indexFromItem(EndPoint *item){
    if(item == rootItem || item == nullptr)
        return QModelIndex();
    EndPoint *parent = item->parentItem();

    QList<EndPoint *> parents;

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

bool EndPointMgr::add(QString id, QString data)
{
//    QDateTime t=QDateTime::currentDateTime();
//    QString timestemp= t.toString("yyyy.dd.MM.hh:mm:ss.zzz");

    //check id exist
    if (isExist(id)){
        QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8());
        QJsonObject jsonObject = doc.object();
        bool update = jsonObject["update"].toBool();
        EndPoint* ep = getEndPoint(id);
        if (update){
            qDebug() << "TODO update EndPointMgr::add: Exist(" << id << ") " << Qt::endl;
            //TODO: do data update!
            ep->loadData(data);
            ep->updateTimeStemp();
        }
        return false;
    } else {
        qDebug() << "EndPointMgr::add: (" << id << ") " << data << Qt::endl;
        //EndPoint* ep = new EndPoint(id, data, this);
        EndPoint* ep = new EndPoint(id, data, rootItem);
        int ibegin = rootItem->childCount();
        int iend = rootItem->childCount()+1;
        QModelIndex midx = indexFromItem(rootItem);
//        qDebug() << "midx:" << midx << " ,begin:" << ibegin << " ,end:" << iend  << Qt::endl;
        beginInsertRows(midx, ibegin, iend);
        rootItem->appendChild(ep);
        endInsertRows();
        ep->updateTimeStemp();
        m_endpoints.append(ep);
        return true;
    }
}

bool EndPointMgr::isExist(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return true;
        }
    }
    return false;
}

int EndPointMgr::getTotalEndpoints()
{
    return m_endpoints.length();
}

EndPoint* EndPointMgr::getEndPoint(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return ep;
        }
    }
    return nullptr;
}
