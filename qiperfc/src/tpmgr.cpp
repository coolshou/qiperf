#include "tpmgr.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QEventLoop>
#include <QWidget>
#include <QPalette>
#include <QColor>
#include <algorithm> // Needed for std::max_element

#include <QDebug>

#include "comm.h"
#include "tpgroup.h"
#include "tp.h"
#include "../src/tpmgrdata.h"
#include "../src/myfunc.h"

TPMgr::TPMgr(int grouptype, QTreeView *treeview, QString tpunit, QObject *parent)
    : QAbstractItemModel(parent), m_tpgrouptype(grouptype), m_treeview(treeview),
      m_TPUint(tpunit)
{
    mDebug = 3;
    connect(this, &QAbstractItemModel::rowsInserted, this, &TPMgr::onRowsInserted);
    m_unit_bits << "Kbits/sec" << "Mbits/sec" << "Gbits/sec" << "Tbits/sec";
    m_unit_bytes << "KBytes/sec" << "MBytes/sec" << "GBytes/sec" << "TBytes/sec";
    m_highestId = 0;
    rootItem = nullptr;
    groupItem = nullptr;
    dirTxItem = nullptr;
    dirRxItem = nullptr;
    // commItem = nullptr;
    //    item = invisibleRootItem();
    reset();
    m_intervals.clear();
    QWidget widget;
    QPalette palette = widget.palette();
    // QFont font = widget.font();
    // font
    m_disabledTextColor = palette.color(QPalette::Disabled, QPalette::Text);
    m_updater = new QTimer();
    connect(m_updater, &QTimer::timeout, this, &TPMgr::onUpdater);
    startUpdater();
}
TPMgr::~TPMgr()
{
    if (groupItem != nullptr)
    {
        delete groupItem;
    }
    if (dirTxItem != nullptr)
    {
        delete dirTxItem;
    }
    if (dirRxItem != nullptr)
    {
        delete dirRxItem;
    }
    if (rootItem != nullptr)
    {
        delete rootItem;
    }
}
QVariant TPMgr::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        if ((index.column() == TP::dir) ||
            (index.column() == TP::throughput) ||
            (index.column() == TP::mintp) ||
            (index.column() == TP::maxtp) ||
            (index.column() == TP::lostrate))
        {
            // align text data to center
            return Qt::AlignCenter;
        }
    }

    TP *item = static_cast<TP *>(index.internalPointer());
    if (role == Qt::ForegroundRole)
    {
        // when item is disabled, grayout text
        if (!item->getEnabled())
        {
            return m_disabledTextColor;
        }
    }
    if (role == Qt::DisplayRole)
    {
        if ((item->getDataType() == TPMgrData::group) ||
            (item->getDataType() == TPMgrData::direction) ||
            (item->getDataType() == TPMgrData::comment) ||
            (item->getDataType() == TPMgrData::config))
        {
            if (item->childCount() > 0)
            {
                if (index.column() == TP::throughput)
                {
                    // sum of subitem's value
                    float sum = 0;
                    QString s;
                    int i = 0;
                    for (TP *child : item->getChilds())
                    {
                        if (child->getThroughput().isEmpty())
                        {
                            i++;
                        }
                        else
                        {
                            sum += child->getThroughput().toFloat();
                        }
                    }
                    if (i == item->getChilds().count())
                    {
                        item->setThroughput("");
                        return QVariant();
                    }
                    else
                    {
                        if (sum > 0)
                        {
                            // if ((item->getDataType()==TPMgrData::group)) {
                            //     qDebug() << "group value: " << sum;
                            // }
                            item->setThroughput(s.setNum(sum));
                            return sum;
                        }
                    }
                }
                if (index.column() == TP::lostrate)
                {
                    // calc Lost Rate
                    int lostpkts = 0;
                    int totalpkts = 0;
                    int i = 0;
                    for (TP *child : item->getChilds())
                    {
                        lostpkts += child->getLostPackets();
                        totalpkts += child->getTotalPackets();
                        i++;
                    }
                    if (totalpkts)
                    {
                        item->setLostRate(QString::number(lostpkts), QString::number(totalpkts));
                    }
                }
            }
            else
            {
                // return QVariant(); //this will return nothing => cell show as empty!!
                // qDebug() << "item ["<< item << "] have no child";
            }
        }
    }
    if ((item->getDataType() == TPMgrData::config) || (item->getDataType() == TPMgrData::group))
    {
        if ((index.column() == TP::throughput) ||
            (index.column() == TP::mintp) ||
            (index.column() == TP::maxtp))
        {
            if (item->data(index.column()).toDouble() <= 0)
            {
                // do not show  value < 0
                return QVariant();
            }
        }
    }
    if (item->getDataType() == TPMgrData::config)
    {
        if (index.column() == TP::dir)
        {
            if (!item->getEnabled())
            {
                // when item disabled, let image grayout too.
                return QVariant("disable" + item->data(index.column()).toString());
            }
            // else { // the column dir will be empty!!
            //      return QVariant();
            //  }
        }
    }

    if (role != Qt::DisplayRole)
    {
        // this will show text data!!
        //  qDebug() << "data not DisplayRole:" << idx ;
        return QVariant();
    }
    return item->data(index.column());
}

Qt::ItemFlags TPMgr::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}
QVariant TPMgr::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
    //    if (role != Qt::DisplayRole)
    //        return QVariant();
    if (role == Qt::TextAlignmentRole)
    {
        if (section != TP::cols::comment)
        {
            // all columns except "comment"
            return Qt::AlignCenter;
        }
    }
    // show header
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case TP::id:
            return QString("idx");
        case TP::server:
            return QString("Server");
        case TP::dir:
            return QString("Direction");
        case TP::client:
            return QString("Client");
        case TP::throughput:
            return QString("TPUT\n(%1)").arg(MyFunc::formatUnit(m_TPUint));
        case TP::mintp:
            return QString("Min\nTPUT");
        case TP::maxtp:
            return QString("Max\nTPUT");
        case TP::lostrate:
            return QString("Lost Rate %\n(lost/total)");
        case TP::comment:
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
        return static_cast<TP *>(parent.internalPointer())->columnCount();
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
        parentItem = static_cast<TP *>(parent.internalPointer());

    TP *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TPMgr::parent(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return QModelIndex();

    TP *childItem = static_cast<TP *>(idx.internalPointer());
    TP *parentItem = childItem->parentItem();
    if (!parentItem)
    {
        // qDebug() << "==== NO parentItem";
        return QModelIndex();
    }
    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TPMgr::rowCount(const QModelIndex &parent) const
{
    // return correct child row count
    TP *parentItem;
    if (parent.column() > 0)
    {
        return 0;
    }

    if (!parent.isValid())
    {
        // Root level: Number of parents
        return rootItem->childCount(); //.size();
    }
    // TODO: groupItem?
    parentItem = static_cast<TP *>(parent.internalPointer());

    // qDebug() << "rowCount parent:" << parentItem << " childCount:" << QString::number(parentItem->childCount());
    return parentItem->childCount();
}

TP *TPMgr::add(QString strJson, QString note, TPMgrData::DataType datatype, TP *parent)
{ // add iperf config item
    TP *pitm = nullptr;
    if (parent)
    {
        pitm = parent;
    }
    else
    {
        pitm = getRootItem(note);
        // qInfo() << "No parent, add:getRootItem " << pitm;
        // qInfo() << "datatype:" << datatype;
        // qInfo() << "TPMgr::add data:" << data;
    }
    QModelIndex midx = indexFromItem(pitm);
    int idx = pitm->childCount();
    beginInsertRows(midx, idx, idx);
    m_highestId = getMaxIdx();
    TP *tp = new TP(QString::number(m_highestId), strJson, datatype, pitm);
    m_tpcfgitems.insert(m_highestId, tp);
    pitm->appendChild(tp);
    endInsertRows();
    return tp;
}
QModelIndex TPMgr::indexFromItem(TP *item)
{
    if (item == rootItem || item == nullptr)
        return QModelIndex();
    TP *parent = item->parentItem();

    QList<TP *> parents;

    while (parent && parent != rootItem)
    {
        parents << parent;
        parent = parent->parentItem();
        if (!parent)
        {
            break;
        }
    }
    QModelIndex ix;
    for (int i = 0; i < parents.count(); i++)
    {
        ix = index(parents[i]->row(), 0, ix);
    }
    ix = index(item->row(), 0, ix);
    return ix;
}

void TPMgr::del(QModelIndex idx)
{
    stopUpdater();
    int row = idx.row();
    TP *itm = getItem(idx);
    int id = itm->getID().toInt();
    qDebug() << " itm:" << itm
             << " TxItm:" << dirTxItem
             << " RxItm:" << dirRxItem;
    QModelIndex pIdx = parent(idx);
    // if (!removeRows(row, 1 , getRootItemIdx())){
    if (!removeRows(row, 1, pIdx))
    {
        qDebug() << "del fail(" << QString::number(row) << "): " << idx;
    }
    if (m_tpcfgitems.contains(id))
    {
        // TODO: should we delete the item?
        m_tpcfgitems.remove(id);
    }

    startUpdater();
}

int TPMgr::rootChildCount()
{
    return rootItem->childCount();
}

QList<TP *> TPMgr::getChilds(bool showAll)
{
    // collect all childs
    // Group: Total+ Iperfs
    // Detail: Iperfs
    // Direction: Tx + Rx + iperfs
    // Comment: x,y,z ... + iperfs
    QList<TP *> tps;
    QList<TP *> items;
    TP *itm;
    // m_tps.clear();
    if (m_tpgrouptype == TPGroup::GroupMode::Total)
    {
        items.append(groupItem);
    }
    else if (m_tpgrouptype == TPGroup::GroupMode::Detail)
    {
        // itm = getRootItem();
        // items.append(itm);
        items.append(dirTxItem);
        items.append(dirRxItem);
    }
    else
    {
        // Direction have two item, Comment may have many
        itm = rootItem;
        for (int i = 0; i < itm->childCount(); i++)
        {
            items.append(itm->child(i));
        }
    }
    if (items.length() > 0)
    {
        for (TP *item : items)
        {
            // qDebug() << " item:" << item << " child:" << item->childCount();
            // qDebug() << " item:" << item->getID();
            if (showAll)
            {
                tps.append(item);
            }
            for (int i = 0; i < item->childCount(); i++)
            {
                TP *chitm = item->child(i);
                if (!showAll)
                {
                    if (!chitm->getEnabled())
                    {
                        continue;
                    }
                }
                // m_tps.append(itm->child(i));
                // qDebug() << " chitm:" << chitm->getID();
                tps.append(chitm);
                if (chitm->haveChilds())
                {
                    for (int j = 0; j < chitm->childCount(); j++)
                    {
                        TP *ccitm = chitm->child(j);
                        // qDebug() << " ccitm:" << ccitm->getID();
                        tps.append(ccitm);
                    }
                }
            }
        }
    }
    // qDebug() << "getChilds tps:" << tps;
    return tps;
}

bool TPMgr::removeRows(int row, int count, const QModelIndex &parent)
{
    TP *parentItem = getItem(parent);
    if (!parentItem)
    {
        parentItem = rootItem;
    }
    bool success = true;
    if (parentItem)
    {
        if (parentItem->childCount() >= (row + count))
        {
            beginRemoveRows(parent, row, row + count - 1);
            success = parentItem->removeChildren(row, count);
            endRemoveRows();
        }
    }
    return success;
}

bool TPMgr::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                     const QModelIndex &destinationParent, int destinationChild)
{
    if (sourceRow < 0 || sourceRow + count > rowCount(sourceParent) ||
        destinationChild < 0 || destinationChild > rowCount(destinationParent))
    {
        return false;
    }
    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    // actual data structure change
    TP *sourceitem = getItem(sourceParent);
    if (!sourceitem)
    {
        sourceitem = rootItem;
    }
    TP *destitem = getItem(destinationParent);
    if (!destitem)
    {
        destitem = rootItem;
    }
    // 从后往前取，避免 index 位移问题
    QList<TP *> moved;
    for (int i = sourceRow + count - 1; i >= sourceRow; i--)
    {
        moved.prepend(sourceitem->takeAt(i)); // 按正序收集
    }
    for (int i = 0; i < moved.count(); i++)
    {
        moved[i]->setParent(destitem);
        destitem->insertChild(destinationChild + i, moved[i]);
    }
    endMoveRows();
    return true;
}

bool TPMgr::moveRow(const QModelIndex &sourceParent, int sourceRow, const QModelIndex &destinationParent, int destinationRow)
{
    TP *sourceParentItem;
    if (!sourceParent.isValid())
    {
        qDebug() << " sourceParent.isValid:" << sourceParent.isValid();
        sourceParentItem = rootItem;
    }
    else
    {
        sourceParentItem = static_cast<TP *>(sourceParent.internalPointer());
    }
    TP *destinationParentItem;
    if (!destinationParent.isValid())
    {
        destinationParentItem = rootItem;
    }
    else
    {
        destinationParentItem = static_cast<TP *>(destinationParent.internalPointer());
    }

    if (sourceRow < 0 || sourceRow > sourceParentItem->childCount() ||
        destinationRow < 0 || destinationRow > destinationParentItem->childCount())
    {
        return false;
    }
    qDebug() << "sourceParent:" << sourceParent << " sourceRow:" << QString::number(sourceRow);
    qDebug() << "destinationParent:" << destinationParent << " destinationRow:" << QString::number(destinationRow);
    beginMoveRows(sourceParent, sourceRow, sourceRow, destinationParent, destinationRow);
    TP *itm = sourceParentItem->child(sourceRow);
    sourceParentItem->removeChild(itm);
    destinationParentItem->insertChild(destinationRow, itm);
    endMoveRows();
    return true;
}

QByteArray TPMgr::savedata()
{
    QJsonArray jsonarr;
    // save all data in json string
    if (rootChildCount() > 0)
    {
        TP *itm = getRootItem();
        for (int row = 0; row < itm->childCount(); ++row)
        {
            TP *tp = itm->child(row);
            QJsonDocument jsonDoc = QJsonDocument::fromJson(tp->saveData().toUtf8());
            QJsonObject jsonObj = jsonDoc.object();
            jsonarr.append(jsonObj);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    else
    {
        qDebug() << "TPMgr::savedata: No data to save";
    }
    QJsonDocument doc(jsonarr);
    return doc.toJson(QJsonDocument::Compact);
}

QStringList TPMgr::getPCs()
{
    QStringList ds;
    // get all config's PC info
    if (this->rootChildCount() > 0)
    {
        TP *itm = getRootItem();
        for (int row = 0; row < itm->childCount(); ++row)
        {
            TP *tp = itm->child(row);
            ds.append(tp->getMgrServer() + ";" + tp->getMgrClient());
        }
    }
    else
    {
        qDebug() << "TPMgr::getPCs: No data to save";
    }
    return ds;
}

bool TPMgr::loaddata(QByteArray data)
{
    // Load data into treeview
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError)
    {
        QJsonArray jsonarr = jsonDoc.array();
        // foreach (const QJsonValue &value, jsonarr) {
        for (const auto value : jsonarr)
        {
            QJsonObject obj = value.toObject();
            QJsonObject o_client = obj["client"].toObject();
            QString note = "Rx";
            if (o_client["reverse"].toBool())
            {
                note = "Tx";
            }
            QJsonDocument doc(obj);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            add(strJson, note);
        }
        return true;
    }
    else
    {
        qDebug() << "TPMgr::loaddata wrong format:(" << error.errorString() << ")\n"
                 << data;
        return false;
    }
}

void TPMgr::reset()
{
    // reset all data to none
    if (groupItem)
    {
        if (groupItem->childCount() > 0)
        {
            groupItem->removeChildren(0, groupItem->childCount());
        }
        // delete groupItem;//direct delete cause app crash??
    }
    if (dirTxItem)
    {
        if (dirTxItem->childCount() > 0)
        {
            dirTxItem->removeChildren(0, dirTxItem->childCount());
        }
    }
    if (dirRxItem)
    {
        if (dirRxItem->childCount() > 0)
        {
            dirRxItem->removeChildren(0, dirRxItem->childCount());
        }
    }
    // if (commItem){
    //     if (commItem->childCount()>0){
    //         commItem->removeChildren(0, commItem->childCount());
    //     }
    // }
    delete rootItem;
    rootItem = new TP(("Root"), ("Root"), TPMgrData::root); //
    // QModelIndex midx = indexFromItem(rootItem);
    // qDebug() << "rootItem:" << rootItem << " midx:" << midx << " valid:" << midx.isValid();
    m_intervals.clear();
    if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Total))
    {
        if (groupItem != nullptr)
        {
            rootItem->appendChild(groupItem);
        }
    }
    if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Direction))
    {
        if (dirTxItem != nullptr)
        {
            rootItem->appendChild(dirTxItem);
        }
        if (dirRxItem != nullptr)
        {
            rootItem->appendChild(dirRxItem);
        }
    }
    // if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Comment)){
    //     if (commItem!=nullptr){
    //         rootItem->appendChild(commItem);
    //     }
    // }
}

void TPMgr::clear()
{
    // clean test record
    stopUpdater();
    // Recursively clear all nodes's throughput data
    std::function<void(TP *)> clearData = [&](TP *node)
    {
        if (!node)
            return;
        for (int i = 0; i < node->childCount(); i++)
        {
            TP *child = node->child(i);
            if (child->getDataType() == static_cast<int>(TPMgrData::config))
            {
                // 找到 config item，清除其下所有 throughput items
                child->clearThroughput();
                if (child->haveChilds())
                {
                    int count = child->childCount();
                    QModelIndex configIndex = indexFromItem(child);
                    beginRemoveRows(configIndex, 0, count - 1);
                    child->removeChildren(0, count);
                    endRemoveRows();
                    emit dataChanged(configIndex, configIndex);
                }
            }
            else
            {
                // group / direction / root ，search in
                child->clearThroughput();
                clearData(child);
            }
        }
        node->clearThroughput();
    };

    // 不管 m_tpgrouptype，直接从 rootItem 开始遍历
    clearData(rootItem);

    m_intervals.clear();
    // Optional: If you modified text/values of 'itm' or 'tp' elements,
    // emit a high-level layout change at the very end instead of processEvents
    emit layoutChanged();
    startUpdater();
}

TP *TPMgr::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TP *item = static_cast<TP *>(index.internalPointer());
        if (item)
        {
            return item;
        }
        else
        {
            qDebug() << "getItem: no item??";
        }
    }
    return nullptr;
}

TP *TPMgr::getRootItem(QString note)
{
    if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Total))
    {
        return getGroupItem();
    }
    else if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Direction))
    {
        return getDirectionItem(note);
    }
    else if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Comment))
    {
        return getCommentItem(note);
    }
    else
    {
        return rootItem;
    }
}

TP *TPMgr::getGroupItem()
{
    if (groupItem)
    {
        return groupItem;
    }
    else
    {
        return newGroupItem();
    }
}

TP *TPMgr::getDirectionItem(QString dir)
{
    if (dir.contains(TPDIRTx))
    {
        if (dirTxItem)
        {
            return dirTxItem;
        }
        else
        {
            return newDirectionItem(dir);
        }
    }
    else if (dir.contains(TPDIRRx))
    {
        if (dirRxItem)
        {
            return dirRxItem;
        }
        else
        {
            return newDirectionItem(dir);
        }
    }
    else
    {
        return rootItem;
    }
}

TP *TPMgr::getCommentItem(QString comm)
{
    qDebug() << "TODO: getCommentItem" << comm;
    return nullptr;
}

QModelIndex TPMgr::getRootItemIdx()
{
    QModelIndex idx;
    idx = indexFromItem(getRootItem());
    qInfo() << "getRootItemIdx:" << idx;
    return idx;
}

void TPMgr::setItem(const QModelIndex &index, TP *item)
{
    if (index.isValid())
    {
        qDebug() << "TODO: setItem:" << index << " item:" << item;
        qDebug() << "Direction:" << item->getDirection();
        //        rootItem->appendChild();
        // TODO: setItem
    }
}

int TPMgr::swapDirection(QModelIndex midx)
{
    QString dir = TPDIRRx;
    TP *tp = getItem(midx);
    if (!tp->isTPDataType())
    {
        log("[swapDirection]Wrong TP datatype: " + midx.data().toString(), 3);
        return 1;
    }
    if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Direction))
    {
        // should change parent item
        TP *oldp;
        TP *newp;
        if (tp->getDirection().contains(TPDIRTx))
        {
            oldp = getDirectionItem(TPDIRTx);
            newp = getDirectionItem(TPDIRRx);
        }
        else
        {
            oldp = getDirectionItem(TPDIRRx);
            newp = getDirectionItem(TPDIRTx);
        }
        QModelIndex srcIdx = indexFromItem(oldp); // source parent
        QModelIndex newIdx = indexFromItem(newp); // destination parent
        if (!newIdx.isValid())
        {
            rootItem->appendChild(newp);
            newIdx = indexFromItem(newp);
        }
        tp->setParent(newp);
        moveRow(srcIdx, tp->row(), newIdx, newp->childCount());
        if (!oldp->haveChilds())
        {
            del(srcIdx);
            oldp = nullptr;
        }
    }
    if (tp->getDirection().contains(TPDIRTx))
    {
        // Tx => Rx
        dir = TPDIRRx;
    }
    else if (tp->getDirection().contains(TPDIRRx))
    {
        // Rx => Tx
        dir = TPDIRTx;
    }
    tp->setDirection(dir);
    if (tp->haveChilds())
    {
        foreach (TP *t, tp->getChilds())
        {
            t->setDirection(dir);
        }
    }
    // 1. If it has children, the easiest way to update the parent row
    //    and all nested rows underneath it is to use layoutChanged().
    if (tp->haveChilds())
    {
        emit layoutChanged();
    }
    // 2. If it's just a single item without children changing,
    //    notifying for its specific row range is more efficient.
    else
    {
        // Find the left-most column and right-most column for this item's row
        QModelIndex topLeft = index(midx.row(), 0, midx.parent());
        QModelIndex bottomRight = index(midx.row(), columnCount() - 1, midx.parent());

        emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, Qt::EditRole});
    }

    return 0;
}

int TPMgr::swapIPDirection(QModelIndex midx)
{
    TP *tp = getItem(midx);
    if (!tp->isTPDataType())
    {
        log("[swapIPDirection]Wrong TP datatype:" + midx.data().toString(), 3);
        return 1;
    }
    QString server = tp->getServer();
    QString mgrServer = tp->getMgrServer();
    QString client = tp->getClient();
    QString mgrclient = tp->getMgrClient();
    tp->swapServerClient(mgrclient, client, mgrServer, server);

    if (midx.isValid())
    {
        // Find the boundary of the row that changed (from column 0 to your last column)
        QModelIndex topLeft = index(midx.row(), 0, midx.parent());
        QModelIndex bottomRight = index(midx.row(), columnCount() - 1, midx.parent());

        // Notify the view to instantly redraw this specific row
        emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, Qt::EditRole});
    }
    return 0;
}

void TPMgr::addComment(QString midx, QString comment)
{
    TP *tp = getItemByIdx(midx);
    if (tp == nullptr)
    {
        qDebug() << "addComment: no parrent iperf pair?? (midx=" << midx << ")";
        return;
    }
    tp->setComment(comment);
    // 1. Convert the TP item into a valid QModelIndex
    QModelIndex itemIndex = indexFromItem(tp);

    if (itemIndex.isValid())
    {
        // 2. Calculate the boundaries for the entire row
        QModelIndex topLeft = index(itemIndex.row(), 0, itemIndex.parent());
        QModelIndex bottomRight = index(itemIndex.row(), columnCount() - 1, itemIndex.parent());

        // 3. Tell the view to repaint the row with the new comment
        emit dataChanged(topLeft, bottomRight, {Qt::DisplayRole, Qt::EditRole});
    }
    else
    {
        // Fallback: If you don't have an indexFromItem function yet,
        // layoutChanged() will force a total view redraw as a temporary fix.
        emit layoutChanged();
    }
}

void TPMgr::addTPdata(QString midx, QString sInterval, QString idx,
                      QString value, QString unit, QString dir,
                      QString pkt_lost, QString pkt_total)
{
    // add throughput item
    Q_UNUSED(sInterval)
    // Q_UNUSED(unit) // TODO: check unit
    if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) != 0)
    {
        qDebug() << "addTPdata Expect unit:" << m_TPUint << " TP unit:" << unit;
    }
    TP *tp = getItemByIdx(midx); // parent item
    if (tp == nullptr)
    {
        return;
    }
    TP *c = getItemByIdx(midx + "_" + idx, tp); // iperf pair config item
    if (c == nullptr)
    {
        // New
        //  beginInsertRows(indexFromItem(tp), 0, 0);
        int iRow = tp->childCount();
        beginInsertRows(indexFromItem(tp), iRow, iRow);
        c = new TP(midx + "_" + idx, "", TPMgrData::TP, tp);
        c->setThroughput(dir, value);
        c->setDirection(dir);
        if (!pkt_lost.isEmpty())
        {
            if (!pkt_total.isEmpty())
            {
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
        tp->appendChild(c); // add iperf pair config item to parent item
        endInsertRows();
    }
    else
    {
        // update throughput value
        c->setThroughput(dir, value);
        if (!pkt_lost.isEmpty())
        {
            if (!pkt_total.isEmpty())
            {
                c->setLostRate(pkt_lost, pkt_total);
            }
        }
    }
}

TP *TPMgr::getItemByIdx(QString midx, TP *item)
{
    QList<TP *> lst;
    if (item == nullptr)
    {
        lst = getChilds(true);
    }
    else
    {
        if (item->haveChilds())
        {
            if (item->childCount() > 0)
            {
                lst = item->getChilds();
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    if (lst.count() > 0)
    {
        foreach (auto tp, lst)
        {
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
            // qDebug() << "tp->getID():" << tp->getID();
            if (tp->getID() == midx)
            {
                return tp;
            }
        }
    }
    else
    {
        qDebug() << "TPMgr::getItemByIdx: No TP list";
    }
    return nullptr;
}

QMap<QString, QStringList> TPMgr::getBindkeys()
{
    // TODO:  get all getBindkeys (managerIP_IP_Port)
    //  each manager qiperfd's IP:Port can not be duplicate
    QMap<QString, QStringList> ds;
    QString mip;
    foreach (auto tp, this->getChilds())
    {
        mip = tp->getMgrServer();
        if (tp->haveChilds())
        {
            foreach (auto ch, tp->getChilds())
            {
                mip.append(ch->getServer() + "_" + QString::number(ch->getPort()));
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    // TODO: what is this??
    return ds;
}

bool TPMgr::isBindkeyExist(QString managerIP, QString bindkey, QModelIndex exc_idx)
{
    // QString mip;
    QString sbindkey;
    foreach (auto tp, this->getChilds())
    {
        if (exc_idx.isValid())
        {
            if (this->getItem(exc_idx) == tp)
            {
                // skip same item
                continue;
            }
        }
        //
        if (tp->getMgrServer().indexOf(managerIP) == 0)
        {
            sbindkey = tp->getServer() + "_" + QString::number(tp->getPort());
            if (sbindkey.indexOf(bindkey) == 0)
            {
                return true;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return false;
}

int TPMgr::getMaxPort(QString m_ip, QString targetIP)
{
    int maxPort = 0;
    int port;
    TP *itm = getRootItem();
    if (itm->haveChilds())
    {
        foreach (auto tp, itm->getChilds())
        {
            if (m_ip == tp->getMgrServer() && (targetIP == tp->getServer()))
            {
                port = tp->getPort();
                if (tp->getProtocal().contains("UDP", Qt::CaseInsensitive))
                {
                    if (tp->getParallel() > 1)
                    {
                        port = port + (tp->getParallel() - 1);
                    }
                }
                if (port > maxPort)
                {
                    maxPort = port;
                }
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    return maxPort;
}

int TPMgr::getMaxIdx()
{
    int maxIdx = 0;
    if (m_tpcfgitems.isEmpty())
    {
        return 0; // Or return -1, depending on your ID logic
    }
    // Finds the maximum key in O(N) time without allocating any memory
    maxIdx = *std::max_element(m_tpcfgitems.keyBegin(), m_tpcfgitems.keyEnd());
    return maxIdx + 1;
}

void TPMgr::onPaste(QString data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isNull())
    {
        QJsonObject jsonRoot = doc.object();
        if (jsonRoot.contains("client") && jsonRoot.contains("server"))
        {
            QJsonObject o_client = jsonRoot["client"].toObject();
            QString note = "Rx";
            if (o_client["reverse"].toBool())
            {
                note = "Tx";
            }
            QJsonObject o_server = jsonRoot["server"].toObject();
            // Get largest iperf port number!!
            int num = getMaxPort(o_server["manager"].toString(),
                                 o_client["target"].toString());
            o_client["port"] = num + 1;
            o_server["port"] = num + 1;
            jsonRoot.remove("client");
            jsonRoot.remove("server");
            jsonRoot.insert("client", o_client);
            jsonRoot.insert("server", o_server);
            doc.setObject(jsonRoot);
            QString strJson(doc.toJson(QJsonDocument::Compact));
            //         qDebug() << "strJson:\n" << strJson;
            add(strJson, note);
        }
        else
        {
            qDebug() << "Wrong format of clipboard data: " << data;
        }
    }
}

void TPMgr::startUpdater()
{
    if (!m_updater->isActive())
    {
        m_updater->start(50); // 0.05 sec, TODO: 0.5 =>  why slow to show up the throughput/lost rate in cfg row??
    }
}

void TPMgr::stopUpdater()
{
    if (m_updater->isActive())
    {
        m_updater->stop();
    }
}

TP *TPMgr::newGroupItem()
{ // create new Total/Group item under rootItem
    QModelIndex midx = indexFromItem(rootItem);
    int idx = rootItem->childCount();
    beginInsertRows(midx, idx, idx);
    groupItem = new TP(GRAPH_TOTAL, GRAPH_TOTAL, TPMgrData::group, rootItem);
    rootItem->appendChild(groupItem);
    endInsertRows();
    return groupItem;
}

TP *TPMgr::newDirectionItem(QString dir)
{
    QModelIndex midx = indexFromItem(rootItem);
    int idx = rootItem->childCount();
    beginInsertRows(midx, idx, idx);
    if (dir.contains(TPDIRTx))
    {
        dirTxItem = new TP(GRAPH_TX, GRAPH_TX, TPMgrData::direction, rootItem);
        rootItem->appendChild(dirTxItem);
    }
    else
    {
        dirRxItem = new TP(GRAPH_RX, GRAPH_RX, TPMgrData::direction, rootItem);
        rootItem->appendChild(dirRxItem);
    }
    endInsertRows();
    if (dir.contains(TPDIRTx))
    {
        return dirTxItem;
    }
    else
    {
        return dirRxItem;
    }
}

void TPMgr::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    double fInterval = sInterval.toDouble();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(datas.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError)
    {
        QJsonArray jArr = doc.array(); //.object();
        // qDebug() << "[TPMgr::onIperfTPdata]refrow(" << refrow << ") sInterval:" << sInterval
        //          << " QJsonArray size:" << jArr.size();
        emit IperfTPdatas(refrow, sInterval, jArr); // for tpplot

        // TODO: this only calc same reporter's value, in --bidir it will have two repoter!!
        //  QHash<double, TPDataGroup> storage;
        //  TPDataGroup storage;
        //  QHash<int, IntervalGroup> storage;
        //  array to hold all -P's rfidx , value & lost rate
        QString dir = QString();
        QString idx;
        QString unit = "";
        QString ssum = "";
        double sum = 0;
        double sum_lost = 0;
        double sum_total = 0;
        double lost_rate = 0;
        bool isAvg = false;
        QString value = "";
        QString slost_rate = "0";
        //        foreach (QJsonObject jObj, jArr){
        for (QJsonArray::const_iterator it = jArr.constBegin(); it != jArr.constEnd(); ++it)
        {
            QJsonObject jObj = it->toObject();
            idx = jObj.value("idx").toString();
            isAvg = jObj.value("AVG").toBool();
            value = jObj.value("value").toString();
            unit = jObj.value("unit").toString();
            // packet lost rate
            QString pkt_lost = jObj.value("packet_lost").toString();
            QString pkt_total = jObj.value("packet_total").toString();

            if (!isAvg)
            {
                if (!jObj.value("dir").isUndefined())
                {
                    dir = jObj.value("dir").toString();
                }
                if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) != 0)
                {
                    qDebug() << "//TODO: base on unit, convert the value to correct value"
                             << " display unit:" << m_TPUint << " tp data unit:" << unit;
                }

                if ((pkt_total.toInt() > 0) && (pkt_lost.toInt() > 0))
                {
                    lost_rate = (pkt_lost.toDouble() / pkt_total.toDouble()) * 100;
                    qDebug() << "TPMgr::onIperfTPdata: lost_rate:" << lost_rate;
                    slost_rate = QString::number(lost_rate, 'f', 4);
                }
                //        qDebug() << "pkt_lost/pkt_total: " << pkt_lost << " / " << pkt_total;
                sum = sum + value.toDouble();
                sum_lost = sum_lost + pkt_lost.toDouble();
                sum_total = sum_total + pkt_total.toDouble();

                if (fInterval >= m_intervals.value(idx, 0.0))
                {
                    addTPdata(refrow, sInterval, idx, value, unit, dir,
                              pkt_lost, pkt_total);
                    m_intervals[idx] = fInterval;
                }
            }
            else
            {
                // this is avg value
                // TODO: this part TP data seems strange??
                qDebug() << "refrow:" << refrow << " idx:" << idx
                         << " sInterval:" + sInterval << " Avg:" << value;
                // << " pkt_lost:" << pkt_lost << " pkt_total:" << pkt_total;
                addTPdata(refrow, sInterval, idx, value, unit, dir,
                          pkt_lost, pkt_total);
                // TODO : each iperf test pair
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }

        if (!isAvg)
        {
            // signal data to tpplot for Group Total
            if (sum_total > 0)
            {
                slost_rate = QString::number((sum_lost / sum_total) * 100, 'f', 4);
            }
            ssum = QString::number(sum, 'f', 3);
            // qDebug() << " Total graph:" << sInterval << " sum:" << ssum
            //          << " lost_rate:" << slost_rate << " dir:" << dir;
            // info update TOTAL plot data
            // emit IperfTPdata(sInterval, GRAPH_TOTAL, ssum, slost_rate, dir);
        }
        // TODO: signal data to tpplot for group Direction
        // TODO: signal data to tpplot for group comment
        // update  test pair config row's sum value

        // iperf test pair
        TP *tp = getItemByIdx(refrow);
        if (!(tp == nullptr))
        {
            if (fInterval >= m_intervals.value(refrow, 0.0))
            {
                // do not update sum when current m_intervals is larger sInterval
                //            qDebug() << "refrow:"<< refrow <<" isAvg:" << isAvg << " sInterval:" << sInterval << " sum:" << sum << " sum lost:" << sum_lost << " sum total:" << sum_total;
                tp->setThroughput(dir, QString::number(sum));
                tp->setLostRate(QString::number(sum_lost), QString::number(sum_total));
                m_intervals[refrow] = fInterval;
            }
        }
        // layoutChanged() will force a total view redraw as a temporary fix.
        emit layoutChanged();
    }
    else
    {
        qDebug() << "TPMgr::onIperfTPdata wrong format:(" << error.errorString() << "\n"
                 << datas;
    }
}

void TPMgr::onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                          QString value, QString unit, QString dir,
                          QString pkt_lost, QString pkt_total)
{
    qDebug() << "onUpdateTPAvg: " << idx << " time:" << sInterval << " : " << value
             << " lost/total:" << pkt_lost << "/" << pkt_total;
    addTPdata(midx, sInterval, idx, value, unit, dir, pkt_lost, pkt_total);
}

QList<TP *> TPMgr::takeConfigsFrom(TP *container)
{
    QList<TP *> configs;
    if (!container)
        return configs;

    // 先收集，避免边遍历边修改
    for (int i = 0; i < container->childCount(); i++)
    {
        if (container->child(i)->getDataType() == static_cast<int>(TPMgrData::config))
        {
            configs.append(container->child(i));
        }
    }
    // 从后往前 take，避免 index 位移
    QModelIndex containerIdx = indexFromItem(container);
    for (int i = configs.count() - 1; i >= 0; i--)
    {
        int row = configs[i]->row();
        beginRemoveRows(containerIdx, row, row);
        container->takeAt(row); // take 不 delete
        endRemoveRows();
    }
    return configs;
}

void TPMgr::insertConfigsTo(TP *target, QList<TP *> configs)
{
    if (!target)
        return;
    QModelIndex targetIdx = indexFromItem(target);
    for (TP *config : configs)
    {
        int row = target->childCount();
        beginInsertRows(targetIdx, row, row);
        config->setParent(target);
        target->appendChild(config);
        endInsertRows();
    }
}

void TPMgr::setTPGroupType(int grouptype)
{
    if (m_tpgrouptype == grouptype)
        return;
    // qDebug() << "old:" << m_tpgrouptype << ", setTPGroupType:" << grouptype;

    // === 1. 收集所有 config items ===
    QList<TP *> configs;
    configs += takeConfigsFrom(rootItem);  // Detail mode
    configs += takeConfigsFrom(groupItem); // Total mode
    configs += takeConfigsFrom(dirTxItem); // Direction mode
    configs += takeConfigsFrom(dirRxItem); // Direction mode

    // === 2. 移除已空的容器 ===
    auto removeIfEmpty = [&](TP *&container)
    {
        if (!container)
            return;
        if (!container->haveChilds())
        {
            int row = container->row();
            QModelIndex rootIdx = indexFromItem(rootItem);
            beginRemoveRows(rootIdx, row, row);
            rootItem->removeChild(container);
            endRemoveRows();
            container = nullptr;
        }
    };
    removeIfEmpty(groupItem);
    removeIfEmpty(dirTxItem);
    removeIfEmpty(dirRxItem);

    // === 3. 更新 grouptype ===
    m_tpgrouptype = grouptype;

    // === 4. 将 config items 分配到新容器 ===
    if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Detail))
    {
        // 直接放 rootItem 下
        insertConfigsTo(rootItem, configs);
    }
    else if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Total))
    {
        // 全部放 groupItem 下
        getGroupItem(); // 不存在则新建
        insertConfigsTo(groupItem, configs);
    }
    else if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Direction))
    {
        // 按 direction 分配到 Tx / Rx
        getDirectionItem(TPDIRTx);
        getDirectionItem(TPDIRRx);
        for (TP *config : configs)
        {
            TP *target = getDirectionItem(config->getDirection());
            insertConfigsTo(target, {config});
        }
    }
    else if (m_tpgrouptype == static_cast<int>(TPGroup::GroupMode::Comment))
    {
        // TODO: comment mode
    }

    emit layoutChanged();
}

void TPMgr::onRowsInserted(const QModelIndex &parent, int first, int last)
{ // when inserted, expand all
    for (; first <= last; ++first)
    {
        m_treeview->expand(this->index(first, 0, parent));
    }
}

void TPMgr::setTPUint(QString tpunit)
{
    if (m_unit_bits.contains(tpunit) || m_unit_bytes.contains(tpunit))
    {
        m_TPUint = tpunit;
    }
    else
    {
        qDebug() << "unknown unit:" << tpunit;
    }
}

QModelIndex TPMgr::setSelectItem(QString idx)
{
    TP *tp = getItemByIdx(idx);
    if (tp)
    {
        QModelIndex midx = indexFromItem(tp);
        return midx;
    }
    else
    {
        qDebug() << "DID not get " << idx << " TP item";
        return QModelIndex();
    }
}

void TPMgr::setDebug(int lv)
{
    mDebug = lv;
}

void TPMgr::onUpdater()
{ // update total throughput/lost rate for each iperf test pair (-P >=1) result
    TP *itm = rootItem;
    if (itm->haveChilds())
    {
        double g_tpvalue = 0.0;
        int g_lostvalue = 0;
        int g_totalvalue = 0;
        for (auto &cfg : itm->getChilds())
        {
            double tpvalue = 0.0;
            int lostvalue = 0;
            int totalvalue = 0;
            for (auto &tp : cfg->getChilds())
            {
                tpvalue = tpvalue + tp->getThroughput().toDouble();
                lostvalue = lostvalue + tp->getLostPackets();
                totalvalue = totalvalue + tp->getTotalPackets();
                // QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            cfg->setThroughput(QString::number(tpvalue));
            cfg->setLostRate(QString::number(lostvalue), QString::number(totalvalue));
            g_tpvalue = g_tpvalue + tpvalue;
            g_lostvalue = g_lostvalue + lostvalue;
            g_totalvalue = g_totalvalue + totalvalue;
        }
        itm->setThroughput(QString::number(g_tpvalue));
        itm->setLostRate(QString::number(g_lostvalue), QString::number(g_totalvalue));
    }
}

void TPMgr::log(QString msg, int lv)
{
    if (lv > mDebug)
    {
        // qDebug() << "[TPMgr]" << msg;
        emit debugMsg("[TPMgr]" + msg);
    }
}

void TPMgr::moveToDirection(TP *tp)
{
    QModelIndex midx = indexFromItem(tp);
    QString dir = tp->getDirection();
    qDebug() << tp->row() << " :current parent:" << tp->parentItem() << " , rootItem:" << rootItem
             << "dir:" << dir << " dirTxItem:" << dirTxItem
             << " dirRxItem:" << dirRxItem;
    if (dir == TPDIRTx)
    {
        // move to Tx Group
        moveRows(midx, midx.row(), 1, indexFromItem(dirTxItem), dirTxItem->childCount());
    }
    else if (dir == TPDIRRx)
    {
        // move to Rx Group
        moveRows(midx, midx.row(), 1, indexFromItem(dirRxItem), dirRxItem->childCount());
    }
    else
    {
        // bidir

        if (tp->parentItem() != rootItem)
        {
            moveRows(midx, midx.row(), 1, indexFromItem(rootItem), rootItem->childCount());
        }
    }
}
