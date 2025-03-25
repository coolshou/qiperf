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
    void addData(double key, double value);
    int getValue(double key, double &value);
    void updateValue(double keyToUpdate, double newvalue);
signals:
    void dataAdded(double key, double value);

};

#endif // MYQCPBARS_H
