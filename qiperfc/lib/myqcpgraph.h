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
    void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
    void addData(double key, double value);
signals:
    void dataAdded(double key, double value);
    void datasSetted(QSharedPointer<QCPGraphDataContainer> data);
};

#endif // MYQCPGRAPH_H
