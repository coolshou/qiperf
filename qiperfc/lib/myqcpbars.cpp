#include "myqcpbars.h"

MyQCPBars::MyQCPBars(QCPAxis *keyAxis, QCPAxis *valueAxis)
    : QCPBars(keyAxis, valueAxis)
{

}

void MyQCPBars::setData(QSharedPointer<QCPBarsDataContainer> data)
{
    QCPBars::setData(data);
}

void MyQCPBars::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
    QCPBars::setData(keys, values, alreadySorted);
    emit datasSetted(data());
}

void MyQCPBars::setData(const QVector<double> &keys, const QVector<int> &lostvalues, const QVector<int> &totalvalues, bool alreadySorted)
{
    QVector<double> values = elementWiseDivision(lostvalues, totalvalues);
    setData(keys, values, alreadySorted);
}

void MyQCPBars::addData(double key, double value)
{
    QCPBars::addData(key, value);
    emit dataAdded(key, value);
}

int MyQCPBars::getValue(double key, double &value)
{
    for (int i=this->dataCount()-1; i>0; i--){
        if (this->data()->at(i)->key == key){
            value = this->data()->at(i)->value;
            return i;
        }
    }
    return -1;
}

void MyQCPBars::updateValue(double keyToUpdate, double newvalue)
{
    QSharedPointer<QCPBarsDataContainer> dataContainer = data();
    // Iterate over the data points to find the specific key
    for (auto it = dataContainer->begin(); it != dataContainer->end(); ++it) {
        if (qFuzzyCompare(it->key, keyToUpdate)) { // Check if the key matches
            it->value = newvalue; // Update the value
            break;
        }
    }
}

QVector<double> MyQCPBars::elementWiseDivision(const QVector<int> &vector1, const QVector<int> &vector2)
{
    QVector<double> result;
    if (vector1.size() != vector2.size()) {
        qDebug() << "Vectors must be of the same size!";
        return result;
    }
    result.reserve(vector1.size()); // Preallocate memory for the result vector

    for (int i = 0; i < vector1.size(); ++i) {
        if (vector2[i] != 0) {
            result.append(vector1[i] / vector2[i]);
        } else {
            qDebug() << "Division by zero at index " << i ;
            result.append(0); // or handle as needed
        }
    }
    return result;
}

