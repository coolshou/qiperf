#ifndef MYFUNC_H
#define MYFUNC_H

// #include <QObject>

#include <QHostAddress>
#include <QAbstractSocket>
#include <QString>

class MyFunc//:public QObject
{
//    Q_OBJECT
public:
    // explicit MyFunc(QObject *parent = nullptr);
    static bool isValidIpAddress(const QString &ip, int &protocal);
    static QString formatUnit(QString val);
};

#endif // MYFUNC_H
