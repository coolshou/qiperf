#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <QObject>
#include <QVariant>
#include <QList>
#include "endpointtype.h"
#include <QJsonObject>

//endpoint store each qiperfd's info
//class EndPoint : public QObject
class EndPoint
{
//    Q_OBJECT
public:
//    explicit EndPoint(const QList<QVariant> &data, EndPoint *parentItem = nullptr);
    explicit EndPoint(QString id, QString data, EndPoint *parentItem = nullptr);
    ~EndPoint() ;//override;

    void appendChild(EndPoint *child);

    EndPoint *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    EndPoint *parentItem();

    //explicit EndPoint(QString id, QString data, QObject *parent = nullptr);

    QString getID();
    void loadData(QString data);
    void updateTimeStemp();
    QString getLastNoticeTime();
signals:

private:
    QList<EndPoint *> m_childItems;
    QList<QVariant> m_itemDatas;
    EndPoint *m_parentItem; //parent item
    EndPointType::Type m_type;
    QString m_id; // reference id, Win/Linux/MacOS: manager IP, android: adb devices id, (TODO: iOS:?)
    QString OS_version; // store OS version
    QString OS_name; // store OS name
    QString m_Manager; // manager interface
    // net (eth/wifi) inerfaces:
    // (TODO)mobile interfaces
    QString m_lastnoticetime; // last get notice time string, eq: 2023.17.06.12:22:07.905
    QJsonObject oNet;
};

#endif // ENDPOINT_H
