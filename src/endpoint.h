#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <QObject>
//endpoint store each qiperfd's info
class EndPoint : public QObject
{
    Q_OBJECT
public:
    explicit EndPoint(QString id, QString data, QObject *parent = nullptr);

    enum Type{
        Unknown=0,
        Windows=1,
        Linux,
        MacOS,
        Android,
        iOS,
        FreeBSD
    };
    Q_ENUM(Type)

    QString getID();
    void loadData(QString data);
    void updateTimeStemp();
    QString getLastNoticeTime();
signals:

private:
    EndPoint::Type m_type;
    QString m_id; // reference id, Win/Linux/MacOS: manager IP, android: adb devices id, (TODO: iOS:?)
    QString OS_version; // store OS version
    QString OS_name; // store OS name
    QString m_Manager; // manager interface
    // net (eth/wifi) inerfaces:
    // (TODO)mobile interfaces
    QString m_lastnoticetime; // last get notice time string, eq: 2023.17.06.12:22:07.905
};

#endif // ENDPOINT_H
