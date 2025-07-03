#ifndef MYQCPBARS_H
#define MYQCPBARS_H

#include "qcustomplot.h"
//#include <QCPBars>
#include <QObject>

class MyQCPBars : public QCPBars
{
    Q_OBJECT
public:
    MyQCPBars(QCPAxis *keyAxis, QCPAxis *valueAxis);
    void setData(QSharedPointer<QCPBarsDataContainer> data);
    void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
    void setData(const QVector<double> &keys, const QVector<int> &lostvalues,
                 const QVector<int> &totalvalues , bool alreadySorted=false);
    void addData(double key, double value);
    int getValue(double key, double &value);
    void updateValue(double keyToUpdate, double newvalue);
    double sumValue(double keyToUpdate, double newvalue);
    void clear();
signals:
    // void dataAdded(double key, double value);
    void datasSetted(QSharedPointer<QCPBarsDataContainer> data);

private:
    QVector<double> elementWiseDivision(const QVector<int>& vector1, const QVector<int>& vector2);
};

#endif // MYQCPBARS_H
