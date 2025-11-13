#include "optimizemodel.h"

#include <QDebug>

OptimizeModel::OptimizeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    OptimizeData rootData= OptimizeData(QDateTime());
    m_rootItem = new OptimizeItem(rootData);
}

OptimizeModel::~OptimizeModel()
{
    delete m_rootItem;
}

QVariant OptimizeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    static const QStringList headers = {
        "Date", "BeamID", "MCS", "RSSI", "SNR",
        "Client\nName", "Client\nBeamID", "Client\nMCS", "Client\nRSSI", "Client\nSNR",
        "UL", "DL"
    };

    return (section >= 0 && section < headers.size()) ? headers[section] : QVariant();

}

bool OptimizeModel::setHeaderData(int section,
                                  Qt::Orientation orientation,
                                  const QVariant &value,
                                  int role)
{
    if (value != headerData(section, orientation, role)) {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

QModelIndex OptimizeModel::index(int row, int column, const QModelIndex &parent) const
{
    OptimizeItem* parentItem = parent.isValid()
                                   ? static_cast<OptimizeItem*>(parent.internalPointer())
                                   : m_rootItem;

    OptimizeItem* childItem = parentItem->child(row);
    return childItem ? createIndex(row, column, childItem) : QModelIndex();
}

QModelIndex OptimizeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();
    OptimizeItem* childItem = static_cast<OptimizeItem*>(index.internalPointer());
    OptimizeItem* parentItem = childItem->parentItem();
    return parentItem == m_rootItem ? QModelIndex() : createIndex(parentItem->row(), 0, parentItem);

}

int OptimizeModel::rowCount(const QModelIndex &parent) const
{
    OptimizeItem* parentItem = parent.isValid()
                                   ? static_cast<OptimizeItem*>(parent.internalPointer())
                                   : m_rootItem;
    return parentItem->childCount();
}

int OptimizeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // if (!parent.isValid())
    //     return 0;
    return COLUMN_COUNT;
}

bool OptimizeModel::hasChildren(const QModelIndex &parent) const
{
    OptimizeItem* parentItem = parent.isValid()
                                   ? static_cast<OptimizeItem*>(parent.internalPointer())
                                   : m_rootItem;

    return parentItem && parentItem->childCount() > 0;
}

bool OptimizeModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    // FIXME: Implement me!
    return false;
}

void OptimizeModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    // FIXME: Implement me!
}

QVariant OptimizeModel::data(const QModelIndex &index, int role) const
{
    if ((role == Qt::TextAlignmentRole) &&
        ((index.column() >= OptimizeColumn::ID))){
        return Qt::AlignCenter;
    }

    if (!index.isValid() || role != Qt::DisplayRole){
        return QVariant();
    }

    const OptimizeItem* item = static_cast<OptimizeItem*>(index.internalPointer());
    const OptimizeData& d = item->data();

    switch (index.column()) {
    case TESTDATE:
        return d.testdate().toString("yyyyMMdd_hhmmss");
    case ID:     {
        if (d.id()>=0){
            return d.id();
        }else{
            return QVariant();
        }
    }
    case MCS:    {
        if (d.mcs()>=0){
            return d.mcs();
        }else{
            return QVariant();
        }
    }
    case RSSI:   {
        if (d.rssi()==0){
            return QVariant();
        }else{
            return d.rssi();
        }
    }
    case SNR:    {
        if (d.snr()==0){
            return QVariant();
        }else{
            return d.snr();
        }
    }
    case C_NAME:  return d.cName();
    case C_ID:  {
        if (d.cId()>=0){
            return d.cId();
        }else{
            return QVariant();
        }
    }
    case C_MCS: {
        if (d.cMcs()>=0){
            return d.cMcs();
        }else{
            return QVariant();
        }
    }
    case C_RSSI: {
        if (d.cRssi()==0){
            return QVariant();
        }else{
            return d.cRssi();
        }
    }
    case C_SNR: {
        if (d.cSnr()==0){
            return QVariant();
        }else{
            return d.cSnr();
        }
    }
    case UL: {
        if (d.ul()>=0){
            return d.ul();
        }else{
            return QVariant();
        }
    }
    case DL: {
        if (d.dl()>=0){
            return d.dl();
        }else{
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
    return QVariant();
}

bool OptimizeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (data(index, role) != value) {
        qDebug() << "// FIXME: Implement setData!";
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

Qt::ItemFlags OptimizeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable; // FIXME: Implement me!
}

bool OptimizeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    qDebug() << "// FIXME: Implement insertRows!";
    endInsertRows();
    return true;
}

bool OptimizeModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    beginInsertColumns(parent, column, column + count - 1);
    qDebug() << "// FIXME: Implement insertColumns!";
    endInsertColumns();
    return true;
}

bool OptimizeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    qDebug() << "// FIXME: Implement removeRows!";
    endRemoveRows();
    return true;
}

bool OptimizeModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    beginRemoveColumns(parent, column, column + count - 1);
    qDebug() << "// FIXME: Implement removeColumns!";
    endRemoveColumns();
    return true;
}

QModelIndex OptimizeModel::addEntry(const OptimizeData &data, const QModelIndex &parentIndex)
{
    OptimizeItem* parentItem = parentIndex.isValid()
                                   ? static_cast<OptimizeItem*>(parentIndex.internalPointer())
                                   : m_rootItem;

    int row = parentItem->childCount();

    beginInsertRows(parentIndex, row, row);
    OptimizeItem* newItem = new OptimizeItem(data, parentItem);
    parentItem->appendChild(newItem);
    endInsertRows();
    return createIndex(row, 0, newItem);  // Return index for column 0
}

QModelIndex OptimizeModel::findEntry(QDateTime testdate)
{
    QModelIndex midx= QModelIndex();
    for(int row=0; row < m_rootItem->childCount(); row++){
        OptimizeItem* citm = m_rootItem->child(row);
        if (citm->getTestDate() == testdate){
            midx = index(row, 0);
            break;
        }
    }

    return midx;
}
