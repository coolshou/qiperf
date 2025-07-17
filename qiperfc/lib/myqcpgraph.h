#ifndef MYQCPGRAPH_H
#define MYQCPGRAPH_H

#include "qcustomplot.h"
// #include <QCPGraph>
#include <QObject>

class MyQCPGraph : public QCPGraph
{
    Q_OBJECT
public:
    MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
    void setData(QSharedPointer<QCPGraphDataContainer> data);
    void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
    void addData(double key, double value);
    int getValueIdx(double key);
    int getValue(double key, double &value);
    void updateValue(double keyToUpdate, double newvalue);
    double sumValue(double keyToUpdate, double newvalue);
    void clear();
    void setInterval(int interval);
    int getInterval();
    double getMaxXValue();
signals:
    // void dataAdded(double key, double value);
    void datasSetted(QSharedPointer<QCPGraphDataContainer> data);
private:
    int m_interval;
};

#endif // MYQCPGRAPH_H
