#ifndef MYFUNC_H
#define MYFUNC_H

// #include <QObject>
#include <QWebSocketProtocol>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QString>

// Version that returns individual components
struct TimeComponents {
    int days;
    int hours;
    int minutes;
    int seconds;
};

class MyFunc//:public QObject
{
//    Q_OBJECT
public:
    // explicit MyFunc(QObject *parent = nullptr);
    static bool isValidIpAddress(const QString &ip, int &protocal);
    static QString formatUnit(QString val);
    static QString secToHumanReadable(long long seconds);
    static TimeComponents secondsToComponents(int totalSeconds);
    static QString closeCodeToString(QWebSocketProtocol::CloseCode code);
    static double calculateFSPL(double distanceMeters, double frequencyHz);
    static double euclideanDistance(double az1, double el1, double az2, double el2);

};

#endif // MYFUNC_H
