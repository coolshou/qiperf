#ifndef MYFUNC_H
#define MYFUNC_H

// #include <QObject>

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
    static QString secToHumanReadable(int seconds);
    static TimeComponents secondsToComponents(int totalSeconds);
};

#endif // MYFUNC_H
