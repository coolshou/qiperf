#include "tpplot.h"

#include <numeric>
#include <QScreen>

TPPlot::TPPlot(bool showgroup, QString sunit, QWidget *parent)
    :QCustomPlot(parent), m_showgroup(showgroup)
{
    //FIXME: under 4K monitor, use setOpenGl(true) cause the TPPlot look over the outer widget width&hight
    // const QList<QScreen*> screens = QGuiApplication::screens();
    // for (const QScreen* screen : screens) {
    //     qDebug() << "Screen: " << screen->name() << " " << screen->size()
    //              << " physicalSize(mm): " << screen->physicalSize()
    //              << " devicePixelRatio:" << screen->devicePixelRatio();
    // }
    m_maxX = 60;
    m_maxY = m_yAxisMaxDefault;
    m_timeWindowThreshold = 120.0;
    //setOpenGl(true); // this will cause plot area looks strange??
    setOpenGl(false);
    setNoAntialiasingOnDrag(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_interval = 1;
    setTPUint(sunit);
    initCustomPlot();
    QCPLayer *mainlayer = layer(LAYER_MAIN);
    if (!addLayer(LAYER_TOTAL, mainlayer, limAbove)){
        qDebug() << "addLayer " << LAYER_TOTAL << " Fail";
    }
    if (!addLayer(LAYER_TOTALOSTRATE, mainlayer, limBelow)){
        qDebug() << "addLayer " << LAYER_TOTALOSTRATE << " Fail";
    }
    if (!addLayer(LAYER_LOSTRATE, layer(LAYER_TOTALOSTRATE), limBelow)){
        // m_LostRateLayer = layer(LAYER_LOSTRATE);
        qDebug() << "addLayer " << LAYER_LOSTRATE << " Fail";
    }
    clear(); // this will let plot layout looks strange!!
    // setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // TODO: init plot chart size not good to fit parent's rect
    m_replottimer = new QTimer();
    connect(m_replottimer, &QTimer::timeout, this, &TPPlot::doReplot);
    // m_replottimer->start(500);//0.5 sec replot
    m_replottimer->start(33);// 大約 30 FPS
}

void TPPlot::setStartTime(QDateTime startTime)
{
    m_starttime = startTime;
}

void TPPlot::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                             QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates)
{
    // qDebug() << "onUpdateTPDatas:" << refrow << " times:" << timedatas << " values: " << valuedatas;
    MyQCPGraph *myGraph = getGraph(refrow);
    connect(myGraph, &MyQCPGraph::datasSetted, this ,&TPPlot::onDatasSetted);

    double minT = *std::min_element(timedatas.begin(), timedatas.end());// x: min time
    double maxT = *std::max_element(timedatas.begin(), timedatas.end());// x: max time
    updateXAxisRange(minT, maxT);

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
        MyQCPBars *g_lostrate = getLostRateGraph(refrow);
        connect(g_lostrate, &MyQCPBars::datasSetted, this ,&TPPlot::onLostRateDatasSetted);
        //lostrate
        Q_UNUSED(lostrates)
        // g_lostrate->setData(timedatas, lostrates);
        g_lostrate->setData(timedatas, packetlosts, packettotals);
    }
    // this->replot();//20260122 tmp remove, use QTimer()
}

void TPPlot::setInterval(int interval)
{
    m_interval = interval;
}

void TPPlot::setShowGroup(bool bShow)
{
    m_showgroup = bShow;
    /* Layer only use to place each graph's layer. not good for setting Visible*/
    // for m_graphs
    QMap<QString, MyQCPGraph*>::const_iterator iterator = m_graphs.constBegin();
    while (iterator != m_graphs.constEnd()) {
        // qDebug() << "graphs:" << iterator.key() << "-" << iterator.value();
        if (iterator.value() != mTotalGraph) {
            // normal graph
            iterator.value()->setVisible(!m_showgroup);
        }else{
            //show total/group graph
            iterator.value()->setVisible(m_showgroup);
            // TODO: update y-axis Max value
        }
        ++iterator;
    }
    // m_lostgraphs
    QMap<QString, MyQCPBars*>::const_iterator lostiterator = m_lostgraphs.constBegin();
    while (lostiterator != m_lostgraphs.constEnd()) {
        // qDebug() << "lostgraphs:" << lostiterator.key() << "-" << lostiterator.value();
        if (lostiterator.value() != mTotalLostGraph) {
            lostiterator.value()->setVisible(!m_showgroup);
        }else{
            lostiterator.value()->setVisible(m_showgroup);
        }
        ++lostiterator;
    }

    //for m_legends
    QMap<QString, QCPAbstractLegendItem*>::const_iterator legenditerator = m_legends.constBegin();
    while (legenditerator != m_legends.constEnd()) {
        // qDebug() << "legenditerator:" << legenditerator.key() << " - " << legenditerator.value() << " - " << mTotalLegendItem; // Why this line cuase app crash??
        if (legenditerator.value() != mTotalLegendItem) {
            //normal Legend
            legenditerator.value()->setVisible(!m_showgroup);
            if (m_showgroup){
                if(legend->hasItem(legenditerator.value())){
                    // qDebug() << "remove legend " << legenditerator.value();
                    if(!legend->take(legenditerator.value())){
                        // qDebug() << "remove legend " << legenditerator.value() << " fail";
                    }else{
                        legend->simplify();
                    }
                }
            }else{
                if(!legend->hasItem(legenditerator.value())){
                    qDebug() << "add legend " <<  legenditerator.value();
                    if(!legend->addItem(legenditerator.value())){
                        qDebug() << "add legend " <<  legenditerator.value() << " fail";
                    }else{
                        legend->simplify();
                    }
                }
            }
        }else{
            //TotalLegend, TODO: check if Totalgraph have data?
            qDebug() << "mTotalGraph->dataCount: " << mTotalGraph->dataCount();
            if (mTotalGraph->dataCount()){
                legenditerator.value()->setVisible(m_showgroup);
            }else {
                legenditerator.value()->setVisible(false);
            }
        }
        ++legenditerator;
    }
    //for m_lostratelegends
    QMap<QString, QCPAbstractLegendItem*>::const_iterator lostlegenditerator = m_lostratelegends.constBegin();
    while (lostlegenditerator != m_lostratelegends.constEnd()) {
        // qDebug() << "lostlegenditerator:" << lostlegenditerator.key() << "-" << lostlegenditerator.value();
        if (lostlegenditerator.value() != mTotalLostLegendItem) {
            //normal lostlegend
            lostlegenditerator.value()->setVisible(!m_showgroup);
            if (m_showgroup){
                if(legend->hasItem(lostlegenditerator.value())){
                    qDebug() << "remove lostlegend " << lostlegenditerator.value();
                    if(!legend->take(lostlegenditerator.value())){
                        qDebug() << "remove lostlegend " << lostlegenditerator.value() << " fail";
                    }else{
                        legend->simplify();
                    }
                }
            }else{
                // qDebug() << "add lostlegenditerator " << lostlegenditerator.value();
                if(!legend->hasItem(lostlegenditerator.value())){
                    qDebug() << "add lostlegend " <<  lostlegenditerator.value();
                    if(!legend->addItem(lostlegenditerator.value())){
                        qDebug() << "add lostlegend " <<  lostlegenditerator.value() << " fail";
                    }else{
                        legend->simplify();
                    }
                }
            }
        }else{
            //TotalLostLegend, TODO: check if TotalLostgraph have data?
            qDebug() << "mTotalLostGraph->dataCount: " << mTotalLostGraph->dataCount();
            if (mTotalLostGraph->dataCount()){
                lostlegenditerator.value()->setVisible(m_showgroup);
            }else{
                lostlegenditerator.value()->setVisible(false);
            }
        }
        ++lostlegenditerator;
    }
    if (m_showgroup){
        //Non Total => Total
        // legend->item()
        if(!legend->hasItem(mTotalLegendItem)){
            if(!legend->addItem(mTotalLegendItem)){
                qDebug() << "add  mTotalLegendItem fail";
            }
        }
        if(!legend->hasItem(mTotalLostLegendItem)){
            if (!legend->addItem(mTotalLostLegendItem)){
                qDebug() << "add  mTotalLostLegendItem fail";
            }
        }
    }else{
        //Total => Non Total
        // TODO: when no data, the plot area will shrink!!
        if(legend->hasItem(mTotalLegendItem)){
            if (!legend->take(mTotalLegendItem)){
                qDebug() << "remove mTotalLegendItem fail";
            }else {
                legend->simplify();
            }
        }
        if(legend->hasItem(mTotalLostLegendItem)){
            if (!legend->take(mTotalLostLegendItem)){
                qDebug() << "remove mTotalLostLegendItem fail";
            }else {
                legend->simplify();
            }
        }
    }
    replot();
}

void TPPlot::setTPUint(QString tpunit)
{
    // conver iperf's unit to display string
    m_TPUint = tpunit;
    QString sunit="Mbps";
    if (tpunit.contains("Kbits")){
        sunit="Kbps";
    }else if (tpunit.contains("Mbits")){
        sunit="Mbps";
    }else if (tpunit.contains("Gbits")){
        sunit="Gbps";
    }else if (tpunit.contains("Tbits")){
        sunit="Tbps";
    }else if (tpunit.contains("KBytes")){
        sunit="KB/s";
    }else if (tpunit.contains("MBytes")){
        sunit="MB/s";
    }else if (tpunit.contains("GBytes")){
        sunit="GB/s";
    }else if (tpunit.contains("TBytes")){
        sunit="TB/s";
    }
    yAxis->setLabel(sunit);
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
        //try calc Total Graph value form each Graphs
        if (mTotalGraph){
            //add all value to Total graph's value

            double orgvalue=0;
            // TODO : last record will be wrong!!??
            QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex, not work
            int rc=mTotalGraph->getValue(key, orgvalue);
            qDebug() << "key:" << QString::number(key)
                     << " orgvalue:" << QString::number(orgvalue)
                     << " new value:" << QString::number(value);
            if (rc>-1){
                //sum up orgvalue & new value
                // qDebug() << "TPPlot::onDataAdded:" << key
                //          << " orgvalue:" << orgvalue
                //          << " value:" << value;
                double sumvalue = orgvalue + value;
                updateYAxisRange(0, sumvalue);
                qDebug() << "key:" << QString::number(key)
                         << " sumvalue:" << QString::number(sumvalue);
                mTotalGraph->updateValue(key,sumvalue);
                mTotalGraph->rescaleAxes(true);
            }else{
                qDebug() << ((MyQCPGraph*)sender())->name() << " (" << key << ")mTotalGraph Key not found";
                mTotalGraph->addData(key,value);
            }

        }else {
            qDebug() << "onDataAdded: ERROR does not have mTotalGraph" ;
        }
    }
    // m_mutex is automatically unlocked when 'locker' goes out of scope here
}

void TPPlot::onDatasSetted(QSharedPointer<QCPGraphDataContainer> data)
{
    //combine two data in to Total graph
    if(mTotalGraph){
        QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex
        QSharedPointer<QCPGraphDataContainer> data1 = mTotalGraph->data();
        QSharedPointer<QCPGraphDataContainer> data2 = data;
        QSharedPointer<QCPGraphDataContainer> sumdata;
        sumdata = sumGraphData(data1, data2);
        mTotalGraph->setData(sumdata);
        mTotalGraph->rescaleAxes(true); //TODO: not good to show y Max value
        if (mTotalLegendItem){
            mTotalLegendItem->setVisible(true);
        }
    }else{
        qDebug() << "onDatasSetted: ERROR does not have mTotalGraph" ;
    }
}

void TPPlot::onLostRateDataAdded(double key, double value)
{
    //LostRate is calc with lost/total packet
    //
    if (m_showgroup){
        if(mTotalLostGraph){
            //add all value to Total graph's value
            double orgvalue=-1;
            QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex
            int rc=mTotalLostGraph->getValue(key, orgvalue);
            if (rc>-1){
                double sumvalue = orgvalue + value;
                // updateYAxisRange(0, sumvalue);
                mTotalLostGraph->updateValue(key,sumvalue);
                mTotalLostGraph->rescaleAxes(true);
            }else{
                qDebug() << ((MyQCPBars*)sender())->name() << " (" << key << ") mTotalLostGraph Key not found";
                mTotalLostGraph->addData(key,value);
            }
        }else {
            qDebug() << "onLostRateDataAdded: ERROR does not have mTotalLostGraph" ;
        }
    }
}

void TPPlot::onLostRateDatasSetted(QSharedPointer<QCPBarsDataContainer> data)
{
    QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex
    if(mTotalLostGraph){
        QSharedPointer<QCPBarsDataContainer> data1 = mTotalLostGraph->data();
        QSharedPointer<QCPBarsDataContainer> data2 = data;
        QSharedPointer<QCPBarsDataContainer> sumdata;
        sumdata = sumLostGraphData(data1, data2);
        mTotalLostGraph->setData(sumdata);
        mTotalLostGraph->rescaleAxes(true); //TODO: not good to show y Max value
        if (mTotalLostLegendItem){
            mTotalLostLegendItem->setVisible(true);
        }
    }else{
        qDebug() << "onLostRateDatasSetted: ERROR does not have mTotalLostGraph" ;
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
            if (item){
                item->setSelected(true);
            }
            if (graph){
                graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
            }
            emit selectedTPitem(graph->name());
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void TPPlot::doReplot()
{
    // 保護機制：確保 Mutex 鎖定，因為我們在讀取可能被 addTPData 修改的變數
    QMutexLocker<QMutex> locker(&m_mutex);
    // 更新 Y 軸：加上一點緩衝空間 (例如 1.1 倍)，視覺上比較舒服
    // if (m_maxY > 0) {
    //     this->yAxis->setRange(0, m_maxY * 1.1);
    // }
    updateYAxisRange(0, m_maxY);

    // 更新 X 軸：典型的「滾動視窗」效果
    // 顯示最新的 m_interval 長度的區間
    // double windowSize = 60.0; // 顯示最近 60 秒
    // this->xAxis->setRange(m_maxX, windowSize, Qt::AlignRight);
    // int minx = 0;
    // if (m_maxX > 60){
    //     minx = m_maxX - 60;
    // }
    // updateXAxisRange(minx, m_maxX);
    updateXAxisRange(0, m_maxX);

    this->replot(QCustomPlot::rpQueuedReplot);
    // this->rpQueuedReplot();
}

void TPPlot::onIperfTPdata(QString sInterval,
                           QString refrowidx, QString data, QString lostrate,
                           QString grouptag)
{
    // double x = sInterval.toDouble();
    int x = (int)sInterval.toDouble(); //ignore .0x Difference of xdata
    double y = data.toDouble();
    Q_UNUSED(grouptag) //TODO grouptag?
    qDebug() << "[TPPlot::onIperfTPdata]:" << refrowidx
             << " sInterval:" << sInterval << " x:" << QString::number(x)
             << " TP:" << QString::number(y);
    //          << " grouptag:" << grouptag;
    addTPData(refrowidx, x, y, lostrate.toDouble());
}

void TPPlot::onIperfTPdatas(QString refrow, QString sInterval, const QJsonArray &dataarray)
{
    QString dir=nullptr;
    QString idx;
    bool isAvg=false;
    QString value="";
    QString unit="";
    QString slost_rate = "0";
    double lost_rate=0;
    for (QJsonArray::const_iterator it=dataarray.constBegin(); it!=dataarray.constEnd(); ++it) {
        QJsonObject jObj= it->toObject();
        idx = jObj.value("idx").toString();
        isAvg = jObj.value("AVG").toBool();
        value = jObj.value("value").toString();
        if (!isAvg) {
            if (!jObj.value("dir").isUndefined()){
                dir=jObj.value("dir").toString();
            }
            unit = jObj.value("unit").toString();
            // if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) !=0){
            //     qDebug() << "//TODO: base on unit, convert the value to correct value"
            //              << " display unit:" << m_TPUint << " tp data unit:" << unit;
            // }
            // packet lost rate
            QString pkt_lost = jObj.value("packet_lost").toString();
            QString pkt_total = jObj.value("packet_total").toString();
            if ((pkt_total.toInt()>0) && (pkt_lost.toInt()>0)){
                lost_rate = (pkt_lost.toDouble()/pkt_total.toDouble())*100;
                qDebug() << "TPMgr::onIperfTPdata: lost_rate:" << lost_rate;
                slost_rate = QString::number(lost_rate, 'f', 4);
            }
        }
        onIperfTPdata(sInterval, refrow+ "_" + idx, value, slost_rate , dir);
    }
}

void TPPlot::addTPData(QString refrowidx, double xdata, double ydata, double lostrate)
{
    {
        QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex,

        double sumydata = ydata;

        // do not double lock in following functions!!, it will cause app hang!!
        MyQCPGraph *myGraph = getGraph(refrowidx);
        //TODO: time window threshold, if we need to scroll back?
        // double lowerBound = xdata - m_timeWindowThreshold;
        // remove old value
        // myGraph->data()->removeBefore(lowerBound); // after exec this, data before lowerBound with not be show
        // store old data
        //show only recent m_timeWindowThreshold data
        xAxis->setRange(xdata, m_timeWindowThreshold, Qt::AlignRight);

        bool isTotal = refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive);
        if (isTotal) {
            //Total Throughput graph
            double oldvalue = 0.0;
            if (myGraph->getValue(xdata, oldvalue)==-1){
                qDebug() << "xdata:" << QString::number(xdata) <<
                    " ydata: " << QString::number(ydata);
                myGraph->addData(xdata, ydata);
            }else {
                sumydata = sumydata + oldvalue;
                qDebug() << "updateValue xdata:" << QString::number(xdata) <<
                    " Total: " << QString::number(sumydata);

                myGraph->updateValue(xdata, sumydata);
            }
        }else{
            //throughput graph
            myGraph->addData(xdata, ydata);
        }
        if (!m_showgroup){
            if (m_legends.contains(refrowidx)){
                QCPAbstractLegendItem *itm = m_legends.value(refrowidx);
                if (itm){
                    itm->setVisible(true);
                }
            }
        } else {
            mTotalLegendItem->setVisible(true);
        }

        // enlarge/shrink y range
        m_maxX = xdata + m_interval;
        if (sumydata>m_maxY){
            m_maxY = sumydata;
        }
        //lost rate
        if (lostrate>0){
            MyQCPBars *g_lostrate = getLostRateGraph(refrowidx);
            if (!refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
                // normal lostrate legends
                g_lostrate->setVisible(!m_showgroup);
            }else{
                mTotalLostLegendItem->setVisible(m_showgroup);
            }
            // // qDebug() << "g_lostrate: " << xdata << " value:" << lostrate;
            // if (m_showgroup){

            // }else {
            //     g_lostrate->setVisible(false);
            // }
            qDebug() << "addTPData, x:" << xdata << " lostrate:" << lostrate;
            g_lostrate->addData(xdata, lostrate);
        }
    }
    // locker.unlock();
    // replot();//20260122 tmp remove, use QTimer()
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


MyQCPGraph *TPPlot::getGraph(QString refrowidx, int width)
{
    QPen graphPen;
    QCPGraph *g;
    MyQCPGraph *myGraph;
    if (!m_graphs.contains(refrowidx)){  // new graphs when not exist
        g = addGraph(xAxis, yAxis);
        // qDebug() << "=====idx:" << idx << " legend->itemCount: " << legend->itemCount();
        // qDebug() << legend->elements(false);
        myGraph = static_cast<MyQCPGraph*>(g);
        int R =rand()%245+10;
        int G =rand()%245+10;
        int B =rand()%245+10;
        graphPen = newColorPen(R, G, B, width);
        myGraph->setPen(graphPen);
        myGraph->setLineStyle(QCPGraph::lsLine);

        if (!refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
            //normal graph
            myGraph->setLayer(LAYER_MAIN);
            if (m_showgroup){
                myGraph->setVisible(false);
            }
            //each throughput graph need to info when data add => to calc total throughput
            // connect(myGraph, &MyQCPGraph::dataAdded, this ,&TPPlot::onDataAdded);
        }else {
            //total graph
            myGraph->setLayer(LAYER_TOTAL);
            if (!m_showgroup){
                myGraph->setVisible(false);
            }
            mTotalGraph = myGraph;
        }
        // qDebug() << "afer addGraph legend Count:" << QString::number(legend->itemCount());
        // legends item
        QCPAbstractLegendItem *litm = legend->item(legend->itemCount()-1);
        if (!m_legends.contains(refrowidx)) {
            // qDebug() << "DO not have m_legend:" << litm << " ADD to m_legends" ;
            m_legends.insert(refrowidx, litm);
        }
        if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){// Total legend
            // qDebug() << "============setup Total legend, m_showgroup:" << m_showgroup;
            // litm->setLayer(LAYER_TOTAL); // DO NOT place Legend in other Layer, it will be Not visible
            mTotalLegendItem = litm;
            // qDebug() << "mTotalLegendItem:" << mTotalLegendItem;
            if (!m_showgroup){
                //when not m_showgroup, the Total graph's legends will take a place in legend
                // qDebug() << "============remove Total legend";
                if (!legend->take(litm)){
                    qDebug() <<"remove mTotalLegendItem:" << mTotalLegendItem << " from legend Fail!!";
                }
            }
            if (myGraph->dataCount()==0){
                litm->setVisible(false);
            }else{
                litm->setVisible(m_showgroup); //total legend item init not Visible
            }
            // myGraph->setVisible(m_showgroup); // graph
        }else{
            // all throughput legend except Total
            // qDebug() << "============setup normal legend";
            // litm->setLayer(LAYER_MAIN);// DO NOT place Legend in other Layer, it will be Not visible
            if (m_showgroup){
                if (!legend->take(litm)){
                    qDebug() <<"remove LegendItem:" << litm << " from legend Fail!!";
                }
            }
            litm->setVisible(!m_showgroup); //legend item
            // myGraph->setVisible(!m_showgroup); // graph
        }
    }else{
        myGraph = m_graphs.value(refrowidx);
        if (!refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
            myGraph->setVisible(!m_showgroup);
        }else{
            myGraph->setVisible(m_showgroup);
            mTotalLegendItem = m_legends.value(refrowidx);
        }
    }
    myGraph->setName(refrowidx);
    if ((!m_graphs.contains(refrowidx))){
        // m_graphs.insert(idx,g);
        m_graphs.insert(refrowidx,myGraph);
        calculateLegendItems();
    }

    return myGraph;
}

MyQCPBars *TPPlot::getLostRateGraph(QString refrowidx)
{
    QPen graphPen;
    QCPGraph *g;
    MyQCPBars *g_lostrate;
    if (!m_lostgraphs.contains(refrowidx)){
        //get main graph's color
        if (m_graphs.contains(refrowidx)){
            // use same color as throughput chart
            g = m_graphs.value(refrowidx);
            graphPen = g->pen();
        } else {
            int R = rand()%245+10;
            int G =rand()%245+10;
            int B =rand()%245+10;
            graphPen = newColorPen(R, G, B, 1);
        }
        QPen redPen = newColorPen(255, 0, 0, 2);
        g_lostrate = new MyQCPBars(xAxis, yAxis2);
        g_lostrate->setName(refrowidx+ " Lost Rate");
        if (!refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){
            //normal lost rate graph
            g_lostrate->setLayer(LAYER_LOSTRATE);
            // connect(g_lostrate, &MyQCPBars::dataAdded, this ,&TPPlot::onLostRateDataAdded);
            if (m_showgroup){
                g_lostrate->setVisible(false);
            }

        }else{
            //total lost rate graph
            g_lostrate->setLayer(LAYER_TOTALOSTRATE);
            mTotalLostGraph = g_lostrate;
            if (!m_showgroup){
                g_lostrate->setVisible(false);
            }
        }
        g_lostrate->setPen(redPen);
        QColor c = graphPen.color();
        c.setAlpha(120); // 0~255, 255 not transparency
        g_lostrate->setBrush(c);

        // qDebug() << "legend->itemCount:" << legend->itemCount();
        QCPAbstractLegendItem *litm = legend->item(legend->itemCount()-1);
        if (!m_lostratelegends.contains(refrowidx)) {
                m_lostratelegends.insert(refrowidx, litm);
        }
        if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){// Total legend
            mTotalLostLegendItem = litm;
            // litm->setLayer(LAYER_TOTALOSTRATE);// DO NOT place Legend in other Layer, it will be Not visible
            // qDebug() << "mTotalLostLegendItem:" << mTotalLostLegendItem;
            if (!m_showgroup){
                if (!legend->take(litm)){
                    qDebug() <<"remove mTotalLostLegendItem:" << mTotalLostLegendItem << " from legend Fail!!";
                }
            }
            if (g_lostrate->dataCount()==0){
                litm->setVisible(false);
            }else{
                litm->setVisible(m_showgroup); //total legend item init not Visible
            }
            // g_lostrate->setVisible(m_showgroup); // graph
        }else{ // all throughput legend except Total
            // litm->setLayer(LAYER_LOSTRATE);// DO NOT place Legend in other Layer, it will be Not visible

            litm->setVisible(!m_showgroup); //legend item
            // g_lostrate->setVisible(!m_showgroup); // graph
            // if (!m_legends.contains(idx)) {
            //     m_legends.insert(idx, litm);
            // }
        }
    }else{
        g_lostrate = m_lostgraphs.value(refrowidx);
        if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)){// Total legend
            mTotalLostLegendItem = m_lostratelegends.value(refrowidx);
        }
    }

    if (!m_lostgraphs.contains(refrowidx)){
        // qDebug() << "m_lostgraphs does not have " << idx << " add lostrate:" << g_lostrate;
        m_lostgraphs.insert(refrowidx,g_lostrate);
        calculateLegendItems();
    }
    return g_lostrate;
}

void TPPlot::clear()
{

    // clearGraphs(); // this will clean all graphs
    if (mTotalGraph){
        mTotalGraph->clear();
        qDebug() << "Reset mTotalGraph";
        mTotalGraph=nullptr;
    }
    // if (mTotalLegendItem){
    //     mTotalLegendItem=nullptr;
    // }
    if (mTotalLostGraph){
        qDebug() << "Reset mTotalLostGraph";
        mTotalLostGraph->clear();
        mTotalLostGraph=nullptr;
    }
    // if (mTotalLostLegendItem){
    //     mTotalLostLegendItem=nullptr;
    // }
    for (auto it = m_graphs.begin(); it != m_graphs.end(); ++it) {
        // disconnect(static_cast<MyQCPGraph*>(it.value()), &MyQCPGraph::dataAdded, this, &TPPlot::onDataAdded);
        removePlottable(it.value());
    }
    m_graphs.clear();
    for (auto it = m_lostgraphs.begin(); it != m_lostgraphs.end(); ++it) {
        // disconnect(static_cast<MyQCPBars*>(it.value()), &MyQCPBars::dataAdded, this, &TPPlot::onLostRateDataAdded);
        removePlottable(it.value());
    }
    m_lostgraphs.clear();
    // m_legends.clear();
    // m_lostratelegends.clear();
    //axis reset
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);

    setStartTime(QDateTime());

    //re-create Total Graph/Total Lost Graph and it's legend
    if (!mTotalGraph){
        mTotalGraph = getGraph(GRAPH_TOTAL, GroupWidth::Total);
    }
    if (!mTotalLostGraph){
        mTotalLostGraph = getLostRateGraph(GRAPH_TOTAL);
    }
    try{
        replot();// when no graph, replot will cause plot area shrink
    }catch (...){
        qDebug() << "TPPlot::clear Unknown ERROR";
    }
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
    // this->setAutoAddPlottableToLegend(true); // when adding a plottable, automatically adds the QCPAbstractLegendItem to the legend
    //x Axis
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    xAxis->setTicker(timeTicker);
    //set axis Label
    xAxis->setLabel("Time(Sec)");
    // yAxis->setLabel(m_tpunit);
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
        QCPLayoutGrid *subLayout = new QCPLayoutGrid();
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

void TPPlot::updateXAxisRange(double mintime, double maxtime)
{
    if (xAxis->range().lower > mintime){
        mintime = xAxis->range().lower;
    }else{
        mintime = mintime *0.9;
    }
    if ((xAxis->range().upper / maxtime)>1.1){
        maxtime = xAxis->range().upper;
    }else{
        maxtime = maxtime *1.1;
    }
    xAxis->setRange(mintime, maxtime);
    // xAxis->setRange(mintime, maxtime, Qt::AlignRight);
    // xAxis->setRange(mintime, maxtime, Qt::AlignCenter);
}

void TPPlot::updateYAxisRange(double minvalue, double maxvalue)
{
    if (yAxis->range().lower < minvalue){
        minvalue = yAxis->range().lower;
    }else{
        minvalue = minvalue - (0.1*minvalue);
    }
    if ((yAxis->range().upper / maxvalue)>1.1){
        maxvalue = yAxis->range().upper;
    }else{
        if (maxvalue < 100){
            // maxvalue = maxvalue + (0.5*maxvalue);
            maxvalue = maxvalue * 1.5;
        }else{
            // maxvalue = maxvalue + (0.1*maxvalue);
            maxvalue = maxvalue * 1.1;
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

    double sumvalue=0.0;
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
            sumvalue =  it1->value + it2->value;
            result->add(QCPGraphData(it1->key, sumvalue));
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

QSharedPointer<QCPBarsDataContainer> TPPlot::sumLostGraphData(const QSharedPointer<QCPBarsDataContainer> &data1, const QSharedPointer<QCPBarsDataContainer> &data2)
{
    QSharedPointer<QCPBarsDataContainer> result(new QCPBarsDataContainer);
    auto it1 = data1->constBegin();
    auto it2 = data2->constBegin();

    while (it1 != data1->constEnd() && it2 != data2->constEnd())
    {
        if (it1->key < it2->key)
        {
            result->add(QCPBarsData(it1->key, it1->value));
            ++it1;
        }
        else if (it1->key > it2->key)
        {
            result->add(QCPBarsData(it2->key, it2->value));
            ++it2;
        }
        else
        {
            result->add(QCPBarsData(it1->key, it1->value + it2->value));
            ++it1;
            ++it2;
        }
    }
    while (it1 != data1->constEnd())
    {
        result->add(QCPBarsData(it1->key, it1->value));
        ++it1;
    }
    while (it2 != data2->constEnd())
    {
        result->add(QCPBarsData(it2->key, it2->value));
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
    // qDebug() << "Approximate number of items that can fit:" << itemsFit;
    if(m_showgroup){
        emit sigLegendCount(1);
    }else{
        qDebug() << "sigLegendCount:" << itemsFit;
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
