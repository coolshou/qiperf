#include "myqcpgraph.h"

MyQCPGraph::MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    :QCPGraph(keyAxis, valueAxis)
{

}

void MyQCPGraph::setData(QSharedPointer<QCPGraphDataContainer> data)
{
    QCPGraph::setData(data);
}

void MyQCPGraph::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
    QCPGraph::setData(keys, values, alreadySorted);
    emit datasSetted(data());
}

void MyQCPGraph::addData(double key, double value)
{
    QCPGraph::addData(key, value);
    // emit dataAdded(key, value);
}

int MyQCPGraph::getValueIdx(double key)
{
    //get key's index
    for (int i=this->dataCount()-1; i>0; i--){
        if (this->data()->at(i)->key == key){
            return i;
        }
    }
    return -1;
}

int MyQCPGraph::getValue(double key, double &value)
{
    //get key's value
    for (int i=this->dataCount()-1; i>0; i--){
        if (this->data()->at(i)->key == key){
            value = this->data()->at(i)->value;
            return i;
        }
    }
    return -1;
}

void MyQCPGraph::updateValue(double keyToUpdate, double newvalue)
{
    QSharedPointer<QCPGraphDataContainer> dataContainer = data();
    // Iterate over the data points to find the specific key
    for (auto it = dataContainer->begin(); it != dataContainer->end(); ++it) {
        if (qFuzzyCompare(it->key, keyToUpdate)) { // Check if the key matches
            it->value = newvalue; // Update the value
            break;
        }
    }
    // parentPlot()->replot();
}

double MyQCPGraph::sumValue(double keyToUpdate, double newvalue)
{
    QSharedPointer<QCPGraphDataContainer> dataContainer = QCPGraph::data();
    // Iterate over the data points to find the specific key
    if (!dataContainer->isEmpty()) {
        // for (auto it = dataContainer->begin(); it != dataContainer->end(); ++it) {
        for (auto it = dataContainer->end(); it != dataContainer->begin();) {
            --it; // Decrement first to get to a valid element
            if (qFuzzyCompare(it->key, keyToUpdate)) { // Check if the key matches
                qDebug() << "key:" << QString::number(keyToUpdate)
                         << " it->value:" << QString::number(it->value)
                         << " newvalue:" << QString::number(newvalue);
                it->value = it->value + newvalue; // Update the value
                return it->value;
            }
        }
    }
    // Not find org key's value, just addd
    addData(keyToUpdate, newvalue);
    return newvalue;

}
