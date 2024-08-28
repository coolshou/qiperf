#ifndef PINGITEM_H
#define PINGITEM_H

#include <QObject>
#include <QVariant>
#include <QList>

class PingItem : public QObject
{
    Q_OBJECT
public:
    enum cols{
        id=0,
        source=1,
        target=2,
        rtime=3,
        minrtime = 4,
        maxrtime = 5,
        lostrate=6,
        comment=7
    };
    Q_ENUM(cols)
    explicit PingItem(QString id, QObject *parent = nullptr);
    PingItem *child(int row);
    PingItem *parentItem();
    QVariant data(int column) const;
    int row() const;
    bool haveChilds();
    int childCount() const;
    int columnCount() const;
    bool getEnabled();

signals:

private:
    QString m_id;
    QList<PingItem *> m_childItems;
    QList<QVariant> m_itemDatas;
    PingItem *m_parentItem;
    bool m_enabled; // enable/disable item
};

#endif // PINGITEM_H
