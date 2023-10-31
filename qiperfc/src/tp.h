#ifndef TP_H
#define TP_H

#include <QObject>
#include <QVariant>
#include <QList>
#include <QJsonObject>


//TP store each throughput config
//class TP : public QObject
class TP
{
    Q_GADGET
public:
//    explicit EndPoint(const QList<QVariant> &data, EndPoint *parentItem = nullptr);
    explicit TP(QString id, QString data, TP *parentItem = nullptr);
//    ~TP() ;//override
    enum DirType{
        Tx=0,
        Rx=1,
        TR=2
    };
    Q_ENUM(DirType)
    enum cols{
        id=0,
        server=1,
        dir=2,
        client=3,
        tp=4,
        comment=5
    };
    Q_ENUM(cols)
//    enum DataField{
//        MServer=0,
//        Dir,
//        MClient,
//        Throughput
//    };
//    Q_ENUM(DataField)
    void appendChild(TP *child);

    TP *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TP *parentItem();
    bool removeChildren(int position, int count);
    //explicit EndPoint(QString id, QString data, QObject *parent = nullptr);

    QString getID();
    void loadData(QString data);
    QString getServer();
    QString getClient();
    QString getMgrServer();
    QString getMgrClient();
    int getPort();
    void updateTimeStemp();
    QString getLastNoticeTime();
signals:

private:
    QList<TP *> m_childItems;
    QList<QVariant> m_itemDatas;
    TP *m_parentItem;
//    EndPointType::Type m_type;
    QString m_jsondata;
    QString m_id; // reference id
    QString m_server; // target server ip
    QString m_mgrserver; // target manger server ip
    QString m_direction; //direction: up/down
    QString m_client; // client ip
    QString m_mgrclient; // manager client ip
    int m_port; //port number
    QString m_tp; // throughput value
    QString m_comment; // comment

    QString m_lastnoticetime; // last get notice time string, eq: 2023.17.06.12:22:07.905

};

#endif // TP_H
