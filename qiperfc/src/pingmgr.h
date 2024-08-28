#ifndef PINGMGR_H
#define PINGMGR_H

#include <QAbstractItemModel>
#include <QObject>
#include <QVariant>

#include "pingitem.h"

class PingMgr : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PingMgr(QObject *parent = nullptr);
    //basic func
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
    PingItem *rootItem;
};

#endif // PINGMGR_H
