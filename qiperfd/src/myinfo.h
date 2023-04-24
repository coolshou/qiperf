#ifndef MYINFO_H
#define MYINFO_H

#include <QJsonObject>
#include <QObject>

class MyInfo : public QObject
{
    Q_OBJECT
public:
    explicit MyInfo(QString mgr_ifname, QObject *parent = nullptr);
    ~MyInfo();

    QString collectInfo();
    QJsonObject collectNetInfo();

signals:

private:
    QString m_ifname;
};

#endif // MYINFO_H
