#ifndef TPMGR_H
#define TPMGR_H

#include <QObject>
#include <QAbstractItemModel>
//#include <QStandardItemModel>
#include <QList>
#include <QJsonObject>
#include <QMap>
#include "tp.h"

class TPStatus: public QObject
{
    Q_GADGET
public:
    enum Status{
        init=0,  // init
        started=1,  // started
        stoped=2     // stoped
    };
    Q_ENUM(Status)
};

class TPMgrData: public QObject
{
    Q_GADGET
public:
    enum DataType{
        root=0,  // root item
        config=1,  // config item
        TP=2     // throughput data item
    };
    Q_ENUM(DataType)
};

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
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    bool add(QString data);
    QModelIndex indexFromItem(TP *item);
    int rootChildCount();
    QList<TP*> getChilds();
    bool removeRows(int position, int rows, const QModelIndex &parent) override;
    QByteArray savedata();
    bool loaddata(QByteArray data);
    void reset();
    void clear();
    TP *getItem(const QModelIndex& index) const;
    TP *getRootItem() const;
    QModelIndex getRootItemIdx();
    void setItem(const QModelIndex& index, TP *item);
    int swapDirection(QModelIndex midx);
    void addComment(QString midx, QString comment);
    void addTPdata(QString midx, QString sInterval, QString idx, QString value, QString unit, QString dir=nullptr);
    TP *getItemByIdx(QString midx, TP *item=nullptr);
    QMap<QString, QStringList> getBindkeys(); // get all getBindkeys (managerIP: IP_Port, IP_Port ...)
    bool isBindkeyExist(QString managerIP, QString bindkey, QModelIndex exc_idx);
    int getMaxPort(QString m_ip, QString targetIP); // get all tp config's port number and return Max value in same manage ip & target ip
    void onPaste(QString data);

public slots:
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
signals:
    void IperfTPdata(QString sInterval, QString idx, QString data);

private:
    TP *rootItem;
    QList<TP*> m_tps; //QList of tp, data


};

#endif // TPMGR_H
