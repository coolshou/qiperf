#ifndef ENDPOINTMGR_H
#define ENDPOINTMGR_H

#include <QObject>
#include <QList>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QJsonArray>
#include <QColor>

#include "../src/endpoint.h"

//class to manager all EndPoint
//class EndPointMgr : public QObject
class EndPointMgr : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum cols{
        name=0,
        ifname=1,
        hostname=2,
        //type=3,
        os=3,
        osver=4,
        cpu=5,
        cpucores=6, //hyper thread number
        status=7,  //last seen
        version=8, //qiperfd version
        buildver=9 //qiperfd git version
    };
    Q_ENUM(cols)

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
    bool addEndpoint(QString id, QString data);
    bool delEndpoint(QString id);
    bool update(QString id, QString data); //update endpoint
    void disable(QString id); //disable endpoint
    bool isExist(QString id);
    int getTotalEndpoints();
    QString getPCsInfo(QStringList pcs);
    QJsonArray getPCsInfos(QStringList pcs);
    QMap<QString, QStringList>  getSerials();

private:
    EndPoint* getEndPoint(QString id);

signals:
private:
//    void setupModelData(const QStringList &lines, EndPoint *parent);
    QStringList _headers;
    EndPoint *rootItem;
    QList<EndPoint*> m_endpoints; //QList of endpoints
    QColor m_disabledTextColor;
};

#endif // ENDPOINTMGR_H
