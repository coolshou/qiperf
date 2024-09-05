#ifndef TPMGR_H
#define TPMGR_H

#include <QObject>
#include <QAbstractItemModel>
//#include <QStandardItemModel>
#include <QList>
#include <QJsonObject>
#include <QMap>
#include <QFileIconProvider>
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
    ////basic read only data model
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    int rowCount(const QModelIndex &parent=QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &idx, int role) const override;
    // // editable data model
    // setData() const override; // require emit dataChanged()
    // flags() const override;  //return ItemIsEditable
//    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    // // header display mathod
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    //setHeaderData() const override;  // require emit headerDataChanged()
    //
//    bool removeRow(int row, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    // ======
    bool add(QString data);
    QModelIndex indexFromItem(TP *item);
    void del(QModelIndex idx);
    int rootChildCount();
    QList<TP*> getChilds();
    QByteArray savedata();
    QStringList getPCs();
    bool loaddata(QByteArray data);
    void reset();
    void clear();
    TP *getItem(const QModelIndex& index) const;
    TP *getRootItem() const;
    QModelIndex getRootItemIdx();
    void setItem(const QModelIndex& index, TP *item);
    int swapDirection(QModelIndex midx);
    int swapIPDirection(QModelIndex midx);
    void addComment(QString midx, QString comment);
    void addTPdata(QString midx, QString sInterval, QString idx,
                   QString value, QString unit, QString dir=nullptr,
                   QString pkt_lost="", QString pkt_total="");
    TP *getItemByIdx(QString midx, TP *item=nullptr);
    QMap<QString, QStringList> getBindkeys(); // get all Bindkeys (managerIP: IP_Port, IP_Port ...)
    bool isBindkeyExist(QString managerIP, QString bindkey, QModelIndex exc_idx);
    int getMaxPort(QString m_ip, QString targetIP); // get all tp config's port number and return Max value in same manage ip & target ip
    void onPaste(QString data);

public slots:
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas);
    void onUpdateTPAvg(QString idx, double time, double value, int packetlost, int packettotal, double lostrate);

signals:
    void IperfTPdata(QString sInterval, QString idx, QString data, QString lostrate);// time, idx, throughput value, lost rate

private:
    TP *rootItem;
    QList<TP*> m_tps; //QList of tp, data
    QFileIconProvider iconProvider;
    QMap<QString, double> m_intervals;
    QColor m_disabledTextColor;
};

#endif // TPMGR_H
