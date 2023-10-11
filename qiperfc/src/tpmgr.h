#ifndef TPMGR_H
#define TPMGR_H

#include <QObject>
#include <QAbstractItemModel>
#include <QList>

#include "tp.h"

//class to manager all Throughput data
class TPMgr : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TPMgr(QObject *parent = nullptr);
    ~TPMgr() override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    int columnCount(const QModelIndex &parent) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    bool add(QString data);
    QModelIndex indexFromItem(TP *item);
    int rootChildCount();
    QList<TP*> getChilds();
    bool removeRows(int position, int rows, const QModelIndex &parent);
private:
//    TP *itemFromIndex(const QModelIndex &index) const;
    TP *getItem(const QModelIndex& index) const;

private:
    TP *rootItem;
    QList<TP*> m_tps; //QList of tp


};

#endif // TPMGR_H
