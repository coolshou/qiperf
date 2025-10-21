#ifndef MYFUNC_H
#define MYFUNC_H

// #include <QObject>
#include <QWebSocketProtocol>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QString>
#include <QtCore/qglobal.h>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

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
    static QStringList getAllIPAddress(bool onlyIPv4=true);
    #if defined(Q_OS_WIN)
    static QString getErrorString(DWORD errorCode);
    #endif
};

#endif // MYFUNC_H
