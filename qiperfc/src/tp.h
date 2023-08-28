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
//    Q_OBJECT
public:
//    explicit EndPoint(const QList<QVariant> &data, EndPoint *parentItem = nullptr);
    explicit TP(QString id, QString data, TP *parentItem = nullptr);
    ~TP() ;//override

    void appendChild(TP *child);

    TP *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    TP *parentItem();

    //explicit EndPoint(QString id, QString data, QObject *parent = nullptr);

    QString getID();
    void loadData(QString data);
    void updateTimeStemp();
    QString getLastNoticeTime();
signals:

private:
    QList<TP *> m_childItems;
    QList<QVariant> m_itemDatas;
    TP *m_parentItem;
//    EndPointType::Type m_type;
    QString m_id; // reference id
    QString m_server; // target server ip
    QString m_direction; //direction: up/down
    QString m_client; // client ip
    QString m_tp; // throughput value
    QString m_comment; // comment

    QString OS_version; // store OS version
    QString OS_name; // store OS name
    QString m_Manager; // manager interface
    // net (eth/wifi) inerfaces:
    // (TODO)mobile interfaces
    QString m_lastnoticetime; // last get notice time string, eq: 2023.17.06.12:22:07.905
    QJsonObject oNet;
};

#endif // TP_H
