#include "myqcpbars.h"

MyQCPBars::MyQCPBars(QCPAxis *keyAxis, QCPAxis *valueAxis)
    : QCPBars(keyAxis, valueAxis)
{

}

void MyQCPBars::addData(double key, double value)
{
    QCPBars::addData(key, value);
    emit dataAdded(key, value);
}
