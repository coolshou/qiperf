#ifndef TPMGR_H
#define TPMGR_H

#include <QObject>
#include <QAbstractItemModel>
#include <QList>
#include <QModelIndex>
#include <QStringList>
#include <QJsonObject>
#include <QMap>
#include <QFileIconProvider>
#include <QColor>
#include <QTimer>
#include <QTreeView>

#include "tp.h"
#include "../src/tpmgrdata.h"

class TPStatus: public QObject
{
    Q_GADGET
public:
    enum Status{
        init=0,  // init
        started=1,  // started
        stoped=2     // stoped
    };
    // init=-1,  // init
    //     started=0,  // started
    //     stoped=1     // stoped
    Q_ENUM(Status)
};


//class to manager all Throughput data
class TPMgr : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TPMgr(bool showgroup=false, QTreeView *treeview=nullptr,  QObject *parent=nullptr);
    ~TPMgr() override;
    // //basic read only data model
    QVariant data(const QModelIndex &idx, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role= Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &idx) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    // // header display mathod
    //setHeaderData() const override;  // require emit headerDataChanged()
//    bool removeRow(int row, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool moveRow(const QModelIndex &sourceParent, int sourceRow,
                 const QModelIndex &destinationParent, int destinationChild) ;
    // ======
    TP * add(QString data, TPMgrData::DataType datatype=TPMgrData::config, TP *parent=nullptr);
    QModelIndex indexFromItem(TP *item);
    void del(QModelIndex idx);
    int rootChildCount();
    QList<TP*> getChilds(bool showAll=true);
    QByteArray savedata();
    QStringList getPCs();
    bool loaddata(QByteArray data);
    void reset();
    void clear();
    TP *getItem(const QModelIndex& index) const;
    TP *getRootItem();
    TP *newGroupItem();
    TP *getGroupItem();

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
    int getMaxIdx();

    void onPaste(QString data);
    void startUpdater();
    void stopUpdater();
    void setTestData();
    QModelIndex setSelectItem(QString idx);

public slots:
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    void onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                       QString value, QString unit, QString dir,
                       QString pkt_lost, QString pkt_total);
    void setShowGroup(bool bShow);
    void onRowsInserted(const QModelIndex &parent, int first, int last);

signals:
    void IperfTPdata(QString sInterval, QString idx, QString data, QString lostrate);// time, idx, throughput value, lost rate

private slots:
    void onUpdater();

private:
    bool m_showgroup;
    QTreeView *m_treeview; //relative treeview
    TP *rootItem;
    TP *groupItem; //hold group item
    // QList<TP*> m_tps; //QList of tp, data
    QFileIconProvider iconProvider;
    QMap<QString, double> m_intervals;
    QColor m_disabledTextColor;
    QTimer *m_updater;
};

#endif // TPMGR_H
