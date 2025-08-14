#include "myqcpgraph.h"

MyQCPGraph::MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    :QCPGraph(keyAxis, valueAxis)
{
    m_interval = 1;
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
    QSharedPointer<QCPGraphDataContainer> dataContainer = data();
    // Iterate over the data points to find the specific key
    if (!dataContainer->isEmpty()) {
        // for (auto it = dataContainer->begin(); it != dataContainer->end(); ++it) {
        for (auto it = dataContainer->end(); it != dataContainer->begin();) {
            --it; // Decrement first to get to a valid element
            if (qFuzzyCompare(it->key, keyToUpdate)) { // Check if the key matches
                // qDebug() << "[MyQCPGraph::sumValue]key:" << QString::number(keyToUpdate)
                //          << " it->value:" << QString::number(it->value)
                //          << " newvalue:" << QString::number(newvalue);
                it->value = it->value + newvalue; // Update the value
                return it->value;
            }
        }
    }
    // Not find org key's value, just add it
    addData(keyToUpdate, newvalue);
    return newvalue;

}

void MyQCPGraph::clear()
{
    // if (dataCount()) { //cause app crash!!
    //     data()->clear();
    // }
}

void MyQCPGraph::setInterval(int interval)
{
    m_interval = interval;
}

int MyQCPGraph::getInterval()
{
    return m_interval;
}

double MyQCPGraph::getMaxXValue()
{
    if (!data().data() || data()->isEmpty()) {
        qDebug() << "[getMaxXValue]Graph has no data.";
        return 0;
    }
    double maxX = -std::numeric_limits<double>::infinity(); // Initialize with negative infinity
    // Iterate through the data points (when many data, this is not good!!)
    // graph->data() returns a QSharedPointer to QCPGraphDataContainer
    // .data() on the QSharedPointer gets the raw pointer
    // *it dereferences the iterator to a QCPGraphData object
    // for (QCPGraphDataContainer::const_iterator it = data()->constBegin(); it != data()->constEnd(); ++it) {
    //     if (it->key > maxX) { // QCPGraph uses 'key' for x-value
    //         maxX = it->key;
    //     }
    // }

    // You can also get the first/last elements if sorted (QCPGraphDataContainer is sorted by key)
    if (!data()->isEmpty()) {
        // qDebug() << "Last (max) X-value from sorted data:" << data()->constEnd().key();
        // Note: constEnd() gives an iterator to one past the last element,
        // so .key() on it should technically be valid for the last element's key.
        // However, it's safer to use the last element directly if you need it.
        // Accessing the last element:
        maxX = (data()->constEnd() - 1)->key;
        // qDebug() << "Last (max) X-value from sorted data directly:" << maxX;
    }
    // qDebug() << "Maximum X-value of the graph:" << maxX;
    return maxX;
}
