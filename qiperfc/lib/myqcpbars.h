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
    void addData(double key, double value);
    int getValue(double key, double &value);
    void updateValue(double keyToUpdate, double newvalue);

signals:
    void dataAdded(double key, double value);
    void datasSetted(QSharedPointer<QCPBarsDataContainer> data);

};

#endif // MYQCPBARS_H
