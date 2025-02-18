#include "myqcpgraph.h"

MyQCPGraph::MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    :QCPGraph(keyAxis, valueAxis)
{

}

void MyQCPGraph::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
    QCPGraph::setData(keys, values, alreadySorted);
    emit datasSetted(data());
}

void MyQCPGraph::addData(double key, double value)
{
    QCPGraph::addData(key, value);
    emit dataAdded(key, value);
}
