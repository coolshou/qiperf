#ifndef TP_H
#define TP_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QJsonObject>


//TP store each throughput config
//class TP : public QObject
class TP: public QObject
{
    Q_GADGET
public:
    explicit TP(QString id, QString data, TP *parentItem = nullptr);
    enum DirType{
        Tx=0,
        Rx=1,
        TR=2,
        RT=3
    };
    Q_ENUM(DirType)
    enum cols{
        id=0,
        server=1,
        dir=2,
        client=3,
        throughput=4,
        lostrate=5,
        comment=6
    };
    Q_ENUM(cols)
    void appendChild(TP *child);
    void clear();
    int findChild(TP *child);

    TP *child(int row);
    QList<TP*> getChilds();
    int childCount() const;
    bool haveChilds();
    int columnCount() const;
    QVariant data(int column) const;
    int setData(int column, QVariant var);
    int row() const;
    TP *parentItem();
    bool removeChildren(int position, int count);
    //explicit EndPoint(QString id, QString data, QObject *parent = nullptr);

    QString getID();
    void loadData(QString data);
    QString getJsonData();
    void resetData();
    QString saveData();
    int getVersion();
    QString getServer();
    QString getServerArgs();
    QString getBindKey(bool s=true);
    QString getClient();
    QString getClientArgs();
    QString getDirection();
    QString getMgrServer(); //manager server ip
    QString getMgrClient(); //manager client ip
    QString getThroughput(); //
    int getWaitTime();
    int setDirection(DirType direction);
    int setDirection(QString direction);
    int getPort();
    void setComment(QString comment);
    void setThroughput(QString value);
    void setThroughput(QString dir, QString value);
    void updateTimeStemp();
    QString getLastNoticeTime();
    void setDataType(int datatype);
    int getDataType();
    QString getTxRxThroughput();
    QString getLostRate();

signals:

private:
    int m_datatype; // item type, 0: init, 1: for config root item, 2: throughput data
    QList<TP *> m_childItems;
    QList<QVariant> m_itemDatas;
    TP *m_parentItem;
//    EndPointType::Type m_type;
    QString m_jsondata;
    QString m_id; // reference id
    int m_version; //iperf version
    QString m_server; // target server ip
    QString m_mgrserver; // target manger server ip
    QString m_direction; //direction: 0,1,2,
    QString m_client; // client ip
    QString m_mgrclient; // manager client ip
    int m_port; //port number
    int m_duration; //test duration in sec
    int m_omit; //test omit time in sec
    QString m_tp; // throughput value
    QString m_comment; // comment

    QString m_lastnoticetime; // last get notice time string, eq: 2023.17.06.12:22:07.905
    double m_Tx; //record Tx throughput
    double m_Rx; //record Rx throughput
    int m_lostpacket; // record lost packet
    int m_totalpacket; // record total packet
};

#endif // TP_H
