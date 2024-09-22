#include "endpointmgr.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFlags>

#include "comm.h"

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
        return QVariant();
    }
//    qDebug() << "data:" << index << " ,role:" << QString::number(role) << Qt::endl;
    EndPoint *item = static_cast<EndPoint*>(index.internalPointer());
    if (index.row()==0 && index.column()==0){

//        qDebug() << "EndPointMgr::data: " << index  << " flasg: " << flags(index) << " value:" << item->data(index.column());
    }

    //    EndPoint *item = itemFromIndex(index);
    return item->data(index.column());
}

Qt::ItemFlags EndPointMgr::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    //    EndPoint *item = static_cast<EndPoint*>(index.internalPointer());
    // TODO: use item's data to change enable/disable status of item
//    if (index.row()==0){
//        Qt::ItemFlags flags(QAbstractItemModel::flags(index));
//        flags = flags & ~Qt::ItemIsEnabled; // disable
//        flags = flags | Qt::ItemIsEnabled; // enable
////        qDebug() << " after flags: " << flags;
//        return flags;
//    }
    return QAbstractItemModel::flags(index);
}

QVariant EndPointMgr::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    // show header
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section)
        {
            case EndPointMgr::name:
                return QString(QIPERFD_NAME);
            case EndPointMgr::ifname:
                return QString("Manager Interface");
            case EndPointMgr::type:
                return QString("Type");
            case EndPointMgr::os:
                return QString("OS");
            case EndPointMgr::osver:
                return QString("OS Ver");
            case EndPointMgr::status:
                return QString("Last seen");
            case EndPointMgr::version:
                return QString("Version");
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
//    parent = rootItem;
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

bool EndPointMgr::add(QString id, QString data)
{
//    QDateTime t=QDateTime::currentDateTime();
//    QString timestemp= t.toString("yyyy.dd.MM.hh:mm:ss.zzz");

    //check id exist
    if (isExist(id)){
        QJsonParseError error;
        QJsonDocument doc= QJsonDocument::fromJson(data.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject jsonObject = doc.object();
    //        bool update = jsonObject["update"].toBool();
            //TODO: update information of endpoint
            EndPoint* ep = getEndPoint(id);
    //        if (update){
    //            qDebug() << "TODO update EndPointMgr::add: Exist(" << id << ") " << Qt::endl;
                //TODO: do data update!
                ep->loadData(data);
                ep->updateTimeStemp();
                //TODO: update UI value from endpoint
                //QModelIndex midx = indexFromItem(ep);
                //qDebug() << "update midx: " << midx << " ep:" << ep;
    //            m_endpoints[id]
    //        }
        }else{
            qDebug() << "wrong format (" << error.errorString() << "\n" << data;
        }
        return false;
    } else {
        //new endpoint
//        qDebug() << "EndPointMgr::add: (" << id << ") " << data;
        EndPoint* ep = new EndPoint(id, data, rootItem);
//        ep->setFlags(Qt::NoItemFlags);
        int ibegin = rootItem->childCount();
        int iend = rootItem->childCount()+1;
        QModelIndex midx = indexFromItem(rootItem);
        beginInsertRows(midx, ibegin, iend);
        rootItem->appendChild(ep);
        endInsertRows();
        ep->updateTimeStemp();
        m_endpoints.append(ep);
        return true;
    }
}

void EndPointMgr::disable(QString id)
{
    if (isExist(id)){
        EndPoint* ep = getEndPoint(id);
        qDebug() << "disable: " << ep->getID() << Qt::endl;

    }
}

bool EndPointMgr::isExist(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return true;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return false;
}

int EndPointMgr::getTotalEndpoints()
{
    return m_endpoints.length();
}

QString EndPointMgr::getPCsInfo(QStringList pcs)
{
    //return json string format of pcs info
    QJsonArray arr=getPCsInfos(pcs);
    QJsonDocument doc;
    doc.setArray(arr);
    // qDebug() << "EndPointMgr::getPCsInfo:" << QString(doc.toJson(QJsonDocument::Compact));
    return QString(doc.toJson(QJsonDocument::Compact));
}

QJsonArray EndPointMgr::getPCsInfos(QStringList pcs)
{
    //return QJsonArray format of pcs info
    QStringList targetpcs;
    //    qDebug() << "getPCsInfo: " << pcs;
    foreach(auto pc, pcs){
        QStringList ds = pc.split(";");
        if (ds.length()==2){
            if (!targetpcs.contains(ds[0])){
                targetpcs.append(ds[0]);
            }
            if (!targetpcs.contains(ds[1])){
                targetpcs.append(ds[1]);
            }
        }else{
            qDebug() << "getPCsInfo: unknown format of pcs: "  << pc;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    qDebug() << "targetpcs:" << targetpcs;

    QStringList targetds;
    foreach(EndPoint *ep, m_endpoints){
        if (targetpcs.contains(ep->getID())){
            //            qDebug() << "ID: " << ep->getID();
            //            qDebug() << "data: " << ep->getJsonData();
            targetds.append(ep->getJsonData());

        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    // qDebug() << "targetds:" << targetds;
    QJsonArray arr= QJsonArray::fromStringList(targetds);
    return arr;
}

EndPoint* EndPointMgr::getEndPoint(QString id)
{
    foreach(EndPoint *ep, m_endpoints){
        if (ep->getID() == id){
            return ep;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return nullptr;
}
