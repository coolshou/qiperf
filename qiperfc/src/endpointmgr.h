#ifndef ENDPOINTMGR_H
#define ENDPOINTMGR_H

#include <QObject>
#include <QList>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "../src/endpoint.h"

//class EndPointMgr : public QObject
class EndPointMgr : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit EndPointMgr(QObject *parent = nullptr);
    ~EndPointMgr() override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex indexFromItem(EndPoint *item);

    bool add(QString id, QString data); //add endpoint
    bool isExist(QString id);
    int getTotalEndpoints();


private:
    EndPoint* getEndPoint(QString id);

signals:
private:
//    void setupModelData(const QStringList &lines, EndPoint *parent);
    QStringList _headers;
    EndPoint *rootItem;

    QList<EndPoint*> m_endpoints; //QList of endpoints
};

#endif // ENDPOINTMGR_H
