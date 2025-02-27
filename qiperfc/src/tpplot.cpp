#include "tpplot.h"

#include <numeric>


TPPlot::TPPlot(bool showgroup, QWidget *parent)
    :QCustomPlot(parent), m_showgroup(showgroup)
{
    m_interval = 1;
    qDebug() << "TPPlot:m_showgroup:" << m_showgroup;
    initCustomPlot();
    clear();

}

void TPPlot::setStartTime(QDateTime startTime)
{
    m_starttime = startTime;
}

void TPPlot::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                             QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates)
{
    // qDebug() << "onUpdateTPDatas:" << refrow << " times:" << timedatas << " values: " << valuedatas;
    QCPGraph *graph = getGraph(refrow);
    MyQCPGraph *myGraph = static_cast<MyQCPGraph*>(graph);
    connect(myGraph, &MyQCPGraph::datasSetted, this ,&TPPlot::onDatasSetted);

    double minT = *std::min_element(timedatas.begin(), timedatas.end());// x: min time
    double maxT = *std::max_element(timedatas.begin(), timedatas.end());// x: max time
    //when have several serial chart, set correct min/max
    // qDebug() << "minRange: " << QString::number(xAxis->range().lower)
    //          << "maxRange: " << QString::number(xAxis->range().upper)
    //          << "minT: " << QString::number(minT)
    //          << "maxT: " << QString::number(maxT);
    if (xAxis->range().lower < minT){
        minT = xAxis->range().lower;
    }
    if (xAxis->range().upper > maxT){
        maxT = xAxis->range().upper;
    }
    minT = minT - (0.1*minT);
    maxT = maxT + (0.1*maxT);
    // xAxis->setRange(minT-30, maxT+30);
    xAxis->setRange(minT, maxT);

    double minV = *std::min_element(valuedatas.begin(), valuedatas.end()); // y: min value
    double maxV = *std::max_element(valuedatas.begin(), valuedatas.end()); // y: max value
    updateYAxisRange(minV, maxV);
    // qDebug() << "refrow:" << refrow << " timedatas: " << timedatas;
    // graph->setData(timedatas, valuedatas);
    myGraph->setData(timedatas, valuedatas);
    // Calculate the sum
    Q_UNUSED(packetlosts)
    int sum = std::accumulate(packettotals.begin(), packettotals.end(), 0);
    if (sum>0){
        // int lost = std::accumulate(packetlosts.begin(), packetlosts.end(), 0);
        QCPBars *g_lostrate = getLostRateGraph(refrow);
        //lostrate
        g_lostrate->setData(timedatas, lostrates);
    }
    // updateTotalGraph();
    this->replot();
}

void TPPlot::setInterval(int interval)
{
    m_interval = interval;
}

void TPPlot::setShowGroup(bool bShow)
{
    m_showgroup = bShow;
    // for m_graphs
    QMap<QString, MyQCPGraph*>::const_iterator iterator = m_graphs.constBegin();
    while (iterator != m_graphs.constEnd()) {
        qDebug() << "graphs:" << iterator.key() << "-" << iterator.value();
        iterator.value()->setVisible(!m_showgroup);
        ++iterator;
    }
    // m_lostgraphs
    QMap<QString, QCPBars*>::const_iterator lostiterator = m_lostgraphs.constBegin();
    while (lostiterator != m_lostgraphs.constEnd()) {
        qDebug() << "lostgraphs:" << lostiterator.key() << "-" << lostiterator.value();
        lostiterator.value()->setVisible(!m_showgroup);
        ++lostiterator;
    }
    // total group graphs
    if (mTotalGraph->dataCount()){
        mTotalGraph->setVisible(m_showgroup);
    }
    // all Legend Item
    //remove first
    if (!m_showgroup){
        if (!legend->take(mTotalLegendItem)){
            qDebug() << "legend take off the mTotalLegendItem " << mTotalLegendItem << " Fail";
        }
    }
    //for m_legends
    QMap<QString, QCPAbstractLegendItem*>::const_iterator legenditerator = m_legends.constBegin();
    while (legenditerator != m_legends.constEnd()) {
        // qDebug() << "legenditerator:" << legenditerator.key() << "-" << legenditerator.value();
        legenditerator.value()->setVisible(!m_showgroup);
        if(m_showgroup){
            if (!legend->take(legenditerator.value())){
                qDebug() << "legend take off the legenditerator " << legenditerator.value() << " Fail";
            }
        }else{
            if (!legend->addElement(legenditerator.value())){
                qDebug() << "legend addElement of legenditerator Fail";
            }
        }
        ++legenditerator;
    }
    //for m_lostratelegends
    QMap<QString, QCPAbstractLegendItem*>::const_iterator lostlegenditerator = m_lostratelegends.constBegin();
    while (lostlegenditerator != m_lostratelegends.constEnd()) {
        qDebug() << "lostlegenditerator:" << lostlegenditerator.key() << "-" << lostlegenditerator.value();
        lostlegenditerator.value()->setVisible(!m_showgroup);
        if(m_showgroup){
            if (!legend->take(lostlegenditerator.value())){
                qDebug() << "legend take off the lostlegenditerator " << lostlegenditerator.value() << " Fail";
            }
        }else{
            if (!legend->addElement(lostlegenditerator.value())){
                qDebug() << "legend addElement of lostlegenditerator Fail";
            }
        }
        ++lostlegenditerator;
    }
    if (mTotalGraph->dataCount()>0){
        mTotalLegendItem->setVisible(m_showgroup);
    }
    if (m_showgroup && (mTotalGraph->dataCount()>0)){
        if (!legend->addElement(mTotalLegendItem)){
            qDebug() << "legend addElement of mTotalLegendItem Fail";
        }
    }
    replot();
}

void TPPlot::onVLegendScrollChanged(int value)
{
    qDebug() << "onVLegendScrollChanged:" << QString::number(value);
    if (!m_showgroup){
        for (int i = 0; i < legend->itemCount(); ++i) {
            QCPAbstractLegendItem *item = legend->item(i);
            item->setVisible(i >= value && i < value + 10); // Show 10 items at a time
        }
        replot();
    }
}

void TPPlot::onDataAdded(double key, double value)
{
    if (m_showgroup){
        if (mTotalGraph){
            double orgvalue=-1;
            int rc=mTotalGraph->getValue(key, orgvalue);
            if (rc>-1){
                double sumvalue = orgvalue + value;
                qDebug() << ((MyQCPGraph*)sender())->name() << " idx:"<< rc << " (" << key << ") orgvalue:" << orgvalue
                         << " sumvalue:" << sumvalue;
                mTotalGraph->updateValue(key,sumvalue);
                mTotalGraph->rescaleAxes(true);
            }else{
                qDebug() << ((MyQCPGraph*)sender())->name() << " (" << key << ") Key not found";
                mTotalGraph->addData(key,value);
            }
            // replot();
        }else {
            qDebug() << "onDataAdded: ERROR does not have mTotalGraph" ;
        }
    }
}

void TPPlot::onDatasSetted(QSharedPointer<QCPGraphDataContainer> data)
{
    if(mTotalGraph){
        QSharedPointer<QCPGraphDataContainer> data1 = mTotalGraph->data();
        QSharedPointer<QCPGraphDataContainer> data2 = data;
        QSharedPointer<QCPGraphDataContainer> sumdata;
        sumdata = sumGraphData(data1, data2);
        mTotalGraph->setData(sumdata);
        mTotalGraph->rescaleAxes(true); //TODO: not good to show y Max value
        mTotalLegendItem->setVisible(true);
    }else{
        qDebug() << "onDatasSetted: ERROR does not have mTotalGraph" ;
    }
}

void TPPlot::selectionChanged()
{
    /* synchronize the selection of the graphs with the selection state of the respective
 legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
 or on its legend item.*/
    // synchronize selection of graphs with selection of corresponding legend items:
    for(int i=0; i < this->graphCount(); i++){
        QCPGraph *graph = this->graph(i);
        QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
        if (((item&&item->selected()) or graph->selected())){
            item->setSelected(true);
            graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
            emit selectedTPitem(graph->name());
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void TPPlot::onIperfTPdata(QString sInterval, QString idx, QString data, QString lostrate)
{
    double x = sInterval.toDouble();
    double y = data.toDouble();
    addTPData(idx, x, y, lostrate.toDouble());
}

void TPPlot::addTPData(QString idx, double xdata, double ydata, double lostrate)
{
//    qDebug() << "addTPData: idx: " << idx;
    // QCPGraph *graph = getGraph(idx);
    MyQCPGraph *myGraph = getGraph(idx);
    // MyQCPGraph *myGraph = static_cast<MyQCPGraph*>(graph);
    if (!idx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
        //throughput graph
        myGraph->addData(xdata, ydata);
        if (!m_showgroup){
            if (m_legends.contains(idx)){
                QCPAbstractLegendItem *itm = m_legends.value(idx);
                if (itm){
                    itm->setVisible(true);
                }
            }
        } else {
            mTotalLegendItem->setVisible(true);
        }
    }
    // enlarge/shrink y range
     updateYAxisRange(0, ydata);
    if (xdata >= xAxis->range().upper){
        xAxis->setRange(0, xdata + m_interval); // add m_interval sec
    }
    //lost rate
    if (lostrate>0){
        QCPBars *g_lostrate = getLostRateGraph(idx);
        // qDebug() << "g_lostrate: " << xdata << " value:" << lostrate;
        if (m_showgroup){

        }else {
            g_lostrate->setVisible(false);
        }
        g_lostrate->addData(xdata, lostrate);
    }
    replot();
}

void TPPlot::del(QString idx)
{
    if (m_lostgraphs.contains(idx)){
        QCPBars *b =  m_lostgraphs.take(idx);
        if (removePlottable(b)){
            //TODO: why actually del but return false?
//                qDebug() << "removePlottable QCPBars fail: " << idx;
        }
    }else{
       // qDebug() << "m_lostgraphs not exist:" << idx;
    }
    if (m_graphs.contains(idx)){
//        QCPGraph *g= m_graphs.value(idx);
        QCPGraph *g= m_graphs.take(idx);
        if (removeGraph(g)){
            //TODO: why actually del but return false?
//            qDebug() << "removeGraph fail: " << idx;
        }
    }else{
        qDebug() << "m_graphs not exist:" << idx;
    }

    replot();
}

// QCPGraph *TPPlot::getGraph(QString idx, int width)
MyQCPGraph *TPPlot::getGraph(QString idx, int width)
{
    QPen graphPen;
    QCPGraph *g;
    MyQCPGraph *myGraph;
    if (!m_graphs.contains(idx)){  // new graphs when not exist
        // qDebug() << "legend Count:" << QString::number(legend->itemCount());
        g = addGraph(xAxis, yAxis);
        myGraph = static_cast<MyQCPGraph*>(g);
        if (!idx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
            if (m_showgroup){
                //each throughput graph need to info when data add => to calc total throughput
                // if (!isSignalConnected(QMetaMethod::fromSignal(&MyQCPGraph::dataAdded))){
                    qDebug() << "getGraph:" << idx << " connect dataAdded of " << myGraph << " ===============";
                    connect(myGraph, &MyQCPGraph::dataAdded, this ,&TPPlot::onDataAdded);
                // }
            }
        }
        // qDebug() << "afer addGraph legend Count:" << QString::number(legend->itemCount());
        int R =rand()%245+10;
        int G =rand()%245+10;
        int B =rand()%245+10;
        graphPen = newColorPen(R, G, B, width);
        myGraph->setPen(graphPen);
        myGraph->setLineStyle(QCPGraph::lsLine);
        // legends item
        QCPAbstractLegendItem *litm = legend->item(legend->itemCount()-1);
        if (idx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){// Total legend
            mTotalLegendItem = litm;
            qDebug() << "mTotalLegendItem:" << mTotalLegendItem;
            if (!m_showgroup){
                // TODO: when not m_showgroup, the Total graph's legends will take a place in legend
                if (!legend->take(litm)){
                    qDebug() <<"remove mTotalLegendItem:" << mTotalLegendItem << " from legend Fail!!";
                }
            }
            // litm->setVisible(m_showgroup); //legend item
            litm->setVisible(false); //total legend item init not Visible
            myGraph->setVisible(m_showgroup); // graph
        }else{ // all throughput legend except Total
            litm->setVisible(!m_showgroup); //legend item
            myGraph->setVisible(!m_showgroup); // graph
            if (!m_legends.contains(idx)) {
                m_legends.insert(idx, litm);
            }
        }

    }else{
        myGraph = m_graphs.value(idx);
    }
    myGraph->setName(idx);
    if (!idx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
        if ((!m_graphs.contains(idx))){
            // m_graphs.insert(idx,g);
            m_graphs.insert(idx,myGraph);
            calculateLegendItems();
        }
    }
    // if (m_showgroup){
    //     emit sigLegendCount(1);
    // }else{
    //     emit sigLegendCount(legend->itemCount());
    // }
    // // calculateLegendItems();
    return myGraph;
}

QCPBars *TPPlot::getLostRateGraph(QString idx)
{
    QPen graphPen;
    QCPGraph *g;
    QCPBars *g_lostrate;
    if (!m_lostgraphs.contains(idx)){
        if (m_graphs.contains(idx)){
            // use same color as throughput chart
            g = m_graphs.value(idx);
            graphPen = g->pen();
        } else {
            int R = rand()%245+10;
            int G =rand()%245+10;
            int B =rand()%245+10;
            graphPen = newColorPen(R, G, B, 1);
        }
        QPen redPen = newColorPen(255, 0, 0, 2);
        g_lostrate = new QCPBars(xAxis, yAxis2);
        g_lostrate->setPen(redPen);
        g_lostrate->setBrush(graphPen.color());
        if (!m_lostratelegends.contains(idx)) {
            QCPAbstractLegendItem *litm = legend->item(legend->itemCount()-1);
            m_lostratelegends.insert(idx, litm);
        }
    }else{
        g_lostrate = m_lostgraphs.value(idx);
    }
    g_lostrate->setName(idx+ " Lost Rate");
    if (!m_lostgraphs.contains(idx)){
        m_lostgraphs.insert(idx,g_lostrate);
        calculateLegendItems();
    }
    // if (m_showgroup){
    //     emit sigLegendCount(1);
    // }else{
    //     emit sigLegendCount(legend->itemCount());
    // }
    return g_lostrate;
}

void TPPlot::clear()
{
    for (auto it = m_graphs.begin(); it != m_graphs.end(); ++it) {

        // removePlottable(it.value());
    }
    clearGraphs(); // this will clean all graphs
    if (mTotalGraph){
        mTotalGraph=nullptr;
    }
    m_graphs.clear();
    for (auto it = m_lostgraphs.begin(); it != m_lostgraphs.end(); ++it) {
        removePlottable(it.value());
    }
    m_lostgraphs.clear();
    m_legends.clear();
    m_lostratelegends.clear();
    // mTotalGraph->data().clear();
    //axis reset
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);

    setStartTime(QDateTime());

    if (!mTotalGraph){
        mTotalGraph = getGraph(GRAPH_TOTAL, 2);
        qDebug() << "mTotalGraph:" << mTotalGraph;
    }
    mTotalGraph->setVisible(m_showgroup);
    //TODO: mTotalGraph's legend will show in legend

    replot();
}

void TPPlot::setXRangeUpper(double upper)
{
    // qDebug() << "setXRangeUpper:" << QString::number(upper);
    xAxis->setRangeUpper(upper);
    replot();
}

void TPPlot::initCustomPlot()
{
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);
    this->axisRect()->setupFullAxesBox();
    // this->setAutoAddPlottableToLegend(false); //true(default): when adding a plottable, automatically adds the QCPAbstractLegendItem to the legend
    //x Axis
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    xAxis->setTicker(timeTicker);

    //set axis Label
    xAxis->setLabel("Time(Sec)");
    yAxis->setLabel("Mbps");
    yAxis2->setLabel("Lost Rate(%)");
    yAxis2->setTickLabels(true);
    //set axis range
    // TODO: update range by throughput/time
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);
    yAxis2->setRange(0, 100);

    // legend
    legend->setVisible(true);
    // connect(legend, &QCPLegend::layerChanged)
    if (1){//TODO: not good on layout
        // Add the QCustomPlot legend to the container
        QCPLayoutGrid *subLayout = new QCPLayoutGrid;
        //TODO: position the legend outside of the graph!!
        plotLayout()->addElement(0, 1, subLayout);
        plotLayout()->setColumnStretchFactor(0, 1); // col 0
        plotLayout()->setColumnStretchFactor(1, 0.1); // col 1
        plotLayout()->setRowStretchFactor(0, 1); // row 0

        subLayout->addElement(0, 0, legend); // row 0, col 0
        //TODO: set legenditem's mini higth?
        // subLayout->addElement(0, 1, vScrollBar);
        // subLayout->addElement(0, 1, new QCPLayoutElement); // row 0 col 1
        // subLayout->addElement(1, 0, new QCPLayoutElement); // row 1 col 0
        subLayout->setColumnStretchFactor(0, 1);
        subLayout->setRowStretchFactor(0, 1);

        QFont legendFont = font();
        legendFont.setPointSize(8);
        legend->setFont(legendFont);
        legend->setSelectedFont(legendFont);
        legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items

        calculateLegendItems();
    }
    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
//    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
    connect(this, &QCustomPlot::selectionChangedByUser, this,  &TPPlot::selectionChanged);
}

QPen TPPlot::newColorPen(int r, int g, int b, int width)
{
    QPen graphPen;
    graphPen.setColor(QColor(r, g, b));
    graphPen.setWidthF(width);
    return graphPen;
}

// void TPPlot::updateTotalGraph()
// {
//     if (m_graphs.values().length()>0){
//         // QSharedPointer<QCPGraphDataContainer> sumData(new QCPGraphDataContainer);
//         QSharedPointer<QCPGraphDataContainer> sumData = mTotalGraph->data();
//         for (auto graph : m_graphs.values()) {
//             QSharedPointer<QCPGraphDataContainer> dataContainer = graph->data();
//             for (auto it = dataContainer->constBegin(); it != dataContainer->constEnd(); ++it) {
//                 auto sumIt = sumData->findBegin(it->key);
//                 // if (!sumIt){

//                 // }
//                 // if((sumIt != sumData->end()) && sumIt){ //found
//                 //     qDebug() << "sumIt:" << sumIt->key << " value:" << sumIt->value;

//                 // }else {  //not found
//                 //     qDebug() << " Did no find sumData key" << it->key;
//                 // }
//                 if (sumIt != sumData->end() && sumIt->key == it->key) {
//                     QCPGraphData updatedData = *sumIt;
//                     updatedData.key = it->key;
//                     qDebug() << graph << " : updatedData.value:" << QString::number(updatedData.value);
//                     updatedData.value += it->value;
//                     sumData->remove(it->key);
//                     sumData->add(updatedData);
//                     // sumIt->value += it->value;
//                 } else {
//                     qDebug() << graph << " new: " <<  QString::number(it->key) << " = "<< QString::number(it->value);
//                     sumData->add(QCPGraphData(it->key, it->value));
//                 }
//             }
//         }
//         qDebug() << "updateTotalGraph:" << sumData ;//<< " size:" << QString::number(sumData.value->size());
//         mTotalGraph->setData(sumData);
//     }else{
//         qDebug() << "updateTotalGraph: No graphs";
//     }
// }

// void TPPlot::updateTotalGraphData(double targetKey, double value)
// {
//     QSharedPointer<QCPGraphDataContainer> dataContainer = mTotalGraph->data();
//     // Iterate through the data and update the value
//     bool keyExists = false;
//     double existingValue = 0.0; // Variable to store the existing value
//     for (auto it = dataContainer->begin(); it != dataContainer->end(); ++it) {
//         if (it->key == targetKey) { // targetKey is the x-value you want to update
//             // it->value = value; // newValue is the new y-value
//             keyExists = true;
//             existingValue = it->value; // Get the existing value
//             it->value = existingValue + value;
//             break;
//         }
//     }
//     if (!keyExists){
//         // Create a new QCPGraphData object with the new key and value
//         QCPGraphData newData;
//         newData.key = targetKey; // newKey is the x-value you want to add
//         newData.value = value; // newValue is the y-value you want to add

//         // Add the new data to the container
//         dataContainer->add(newData);
//     }
//     replot();
// }

void TPPlot::updateYAxisRange(double minvalue, double maxvalue)
{
    if (yAxis->range().lower < minvalue){
        minvalue = yAxis->range().lower;
    }else{
        minvalue = minvalue - (0.1*minvalue);
    }
    if (yAxis->range().upper > maxvalue){
        maxvalue = yAxis->range().upper;
    }else{
        if (maxvalue < 100){
            maxvalue = maxvalue + (0.5*maxvalue);
        }else{
            maxvalue = maxvalue + (0.1*maxvalue);
        }
    }
    yAxis->setRange(minvalue, maxvalue);
}

QVector<QCPGraphData> TPPlot::convertQMapToQVector(const QMap<double, double> &map)
{
    // Not good for many QCPGraph data in!!
    //TODO: when many data, is this good in preformance?
    QVector<QCPGraphData> vector;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it)
    {
        QCPGraphData data;
        data.key = it.key();
        data.value = it.value();
        vector.append(data);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return vector;
}

QSharedPointer<QCPGraphDataContainer> TPPlot::sumGraphData(const QSharedPointer<QCPGraphDataContainer> &data1, const QSharedPointer<QCPGraphDataContainer> &data2)
{
    QSharedPointer<QCPGraphDataContainer> result(new QCPGraphDataContainer);
    auto it1 = data1->constBegin();
    auto it2 = data2->constBegin();

    while (it1 != data1->constEnd() && it2 != data2->constEnd())
    {
        if (it1->key < it2->key)
        {
            result->add(QCPGraphData(it1->key, it1->value));
            ++it1;
        }
        else if (it1->key > it2->key)
        {
            result->add(QCPGraphData(it2->key, it2->value));
            ++it2;
        }
        else
        {
            result->add(QCPGraphData(it1->key, it1->value + it2->value));
            ++it1;
            ++it2;
        }
    }
    while (it1 != data1->constEnd())
    {
        result->add(QCPGraphData(it1->key, it1->value));
        ++it1;
    }
    while (it2 != data2->constEnd())
    {
        result->add(QCPGraphData(it2->key, it2->value));
        ++it2;
    }

    return result;
}

void TPPlot::calculateLegendItems()
{
    //get the legend size and calculate the number of items can show
    QSize legendSize = legend->rect().size();//->minimumOuterSizeHint();
    //calculate Max Legend Items
    int itemHeight = legend->font().pointSize() + 9; // approximate height of each item
    int itemsFit = legendSize.height() / itemHeight;

    // qDebug() << "Legend size:" << legendSize.height() << " itemHeight:" << itemHeight;
    // qDebug() << "Legend rect:" << legend->rect();
    qDebug() << "Approximate number of items that can fit:" << itemsFit;
    if(m_showgroup){
        emit sigLegendCount(1);
    }else{
        emit sigLegendCount(itemsFit);
    }
}

QCPDataContainer<QCPGraphData>::const_iterator TPPlot::findKeyValue(const QCPDataContainer<QCPGraphData> &container, double key)
{
    // Use findBegin to get an iterator to the data point with the closest key
    auto it = container.findBegin(key, true);
    // if (it != container.constEnd() && it->key == key) {
    if (it != container.constEnd()) {
        qDebug() << "it->key:" << it->key << " key:" << key;
        if (it->key == key){
            return it; // Found the exact key
        }
    }
    return container.constEnd(); // Key not found
}
