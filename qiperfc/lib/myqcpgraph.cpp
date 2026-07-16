#include "myqcpgraph.h"

MyQCPGraph::MyQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis)
    :QCPGraph(keyAxis, valueAxis)
{
    m_interval = 1;
    m_dir = -1;
    setAdaptiveSampling(true);
}

void MyQCPGraph::setData(QSharedPointer<QCPGraphDataContainer> data)
{
    QCPGraph::setData(data);
}

void MyQCPGraph::setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted)
{
    QCPGraph::setData(keys, values, alreadySorted);
    emit datasSetted(data(), m_dir);
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
        if (qFuzzyCompare(this->data()->at(i)->key , key)){
            return i;
        }
    }
    return -1;
}

int MyQCPGraph::getValue(double key, double &value)
{
    // 1. 檢查防禦：如果根本沒資料，直接回傳 -1
    int count = this->dataCount();
    if (count == 0) return -1;

    //get key's value
    for (int i = count - 1; i >= 0; i--){
        // if (qFuzzyCompare(this->data()->at(i)->key , key)){
        if (qAbs(this->data()->at(i)->key - key) < 0.0001){
            //found key, store value to value
            value = this->data()->at(i)->value;
            return i;
        }
    }
    return -1;
}

void MyQCPGraph::updateValue(double keyToUpdate, double newvalue)
{
    // 取得非 const 的數據容器指標
    QSharedPointer<QCPGraphDataContainer> dataContainer = data();
    if (!dataContainer || dataContainer->isEmpty()) return;

    int count = dataContainer->size();
    // 同樣從最後一個點倒著往前找
    for (int i = count - 1; i >= 0; i--)
    {
        // 使用 dataContainer->data()->投射出可修改的疊代器或直接存取
        // 注意：QCPGraphDataContainer 內部是用 std::vector 或 QVector 儲存
        // 我們可以透過 begin() + i 拿到該位置的非 const 疊代器
        auto it = dataContainer->begin() + i;

        if (qAbs(it->key - keyToUpdate) < 0.0001)
        {
            // 透過非 const 疊代器修改數值，這樣就不會報唯讀錯誤了！
            it->value = newvalue;
            break;
        }
    }
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

void MyQCPGraph::setDirection(int iDir)
{
    m_dir = iDir;
}

int MyQCPGraph::getDirection()
{
    return m_dir;
}
