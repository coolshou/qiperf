#include "tpplot.h"

#include <numeric>

TPPlot::TPPlot(QWidget *parent):QCustomPlot(parent)
{
    initCustomPlot();
}

void TPPlot::setStartTime(QDateTime startTime)
{
    m_starttime = startTime;

}

void TPPlot::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                             QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate)
{
    qDebug() << "onUpdateTPDatas:" << refrow;
    QCPGraph *graph = getGraph(refrow);

    double minT = *std::min_element(timedatas.begin(), timedatas.end());
    double maxT = *std::max_element(timedatas.begin(), timedatas.end());
    xAxis->setRange(minT-30, maxT+30);

    double minV = *std::min_element(valuedatas.begin(), valuedatas.end());
    double maxV = *std::max_element(valuedatas.begin(), valuedatas.end());
    yAxis->setRange(minV*0.9, maxV*1.1);

    graph->setData(timedatas, valuedatas);
    //TODO: lostrate

    // Calculate the sum
    int sum = std::accumulate(packettotals.begin(), packettotals.end(), 0);
    if (sum>0){
        int lost = std::accumulate(packetlosts.begin(), packetlosts.end(), 0);
        qDebug() << "lost: " << QString::number(lost) << " total: " << QString::number(sum);
        QCPBars *g_lostrate = getLostRateGraph(refrow);
        g_lostrate->setData(timedatas, lostrate);
    }
    this->replot();
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
    QCPGraph *graph = getGraph(idx);
    // enlarge/shrink y range
    if ((ydata >= yAxis->range().upper) ){
        int aval;
        if (ydata < 100){
            aval= round(ydata*1.5);
        }else{
            aval= round(ydata*1.1);
        }
        yAxis->setRange(0, aval);
    }
    if (xdata >= xAxis->range().upper){
        xAxis->setRange(0, xdata+30);
    }
    // add x/y data to graphic
    graph->addData(xdata, ydata);
    //lost rate
    if (lostrate>0){
        QCPBars *g_lostrate = getLostRateGraph(idx);
        qDebug() << "g_lostrate: " << xdata << " value:" << lostrate;
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
       qDebug() << "QCPBars not exist:" << idx;
    }
    if (m_graphs.contains(idx)){
//        QCPGraph *g= m_graphs.value(idx);
        QCPGraph *g= m_graphs.take(idx);
        if (removeGraph(g)){
            //TODO: why actually del but return false?
//            qDebug() << "removeGraph fail: " << idx;
        }
    }else{
        qDebug() << "QCPGraph not exist:" << idx;
    }

    replot();
}

QCPGraph *TPPlot::getGraph(QString idx)
{
    QPen graphPen;
    QCPGraph *g;
    if (!m_graphs.contains(idx)){
        g = addGraph(xAxis, yAxis);
        int R = rand()%245+10;
        int G =rand()%245+10;
        int B =rand()%245+10;
        graphPen = newColorPen(R, G, B, 1);
        g->setPen(graphPen);
        g->setLineStyle(QCPGraph::lsLine);
    }else{
        g = m_graphs.value(idx);
    }
//    qDebug() << "getGraph: " << idx << " g:" << g;
    g->setName(idx);
    if (!m_graphs.contains(idx)){
        m_graphs.insert(idx,g);
    }
    return g;
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
        g_lostrate = new QCPBars(xAxis, yAxis2);
        g_lostrate->setPen(graphPen);
        g_lostrate->setBrush(graphPen.color());

    }else{
        g_lostrate = m_lostgraphs.value(idx);
    }
    g_lostrate->setName(idx+ " Lost Rate");
    if (!m_lostgraphs.contains(idx)){
        m_lostgraphs.insert(idx,g_lostrate);
    }
    return g_lostrate;
}

void TPPlot::clear()
{
    clearGraphs();
    m_graphs.clear();
    for (auto it = m_lostgraphs.begin(); it != m_lostgraphs.end(); ++it) {
        removePlottable(it.value());
    }
    m_lostgraphs.clear();
    //axis reset
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);

    setStartTime(QDateTime());

    replot();
}

void TPPlot::initCustomPlot()
{
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                                  QCP::iSelectLegend | QCP::iSelectPlottables);
    this->axisRect()->setupFullAxesBox();
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

//    this->replot();
    // legend
    legend->setVisible(true);
    if (1){//TODO: not good on layout
        // Add the QCustomPlot legend to the container
        QCPLayoutGrid *subLayout = new QCPLayoutGrid;
        //TODO: position the legend outside of the graph!!
        //
        plotLayout()->addElement(0, 1, subLayout);
        plotLayout()->setColumnStretchFactor(0, 1);
        plotLayout()->setColumnStretchFactor(1, 0.1); // col 1
        plotLayout()->setRowStretchFactor(0, 1); // row 0
        subLayout->addElement(0, 0, legend); // row 0
        subLayout->addElement(0, 1, new QCPLayoutElement); // row 0 col 0
        subLayout->addElement(1, 0, new QCPLayoutElement); // row 1
        subLayout->setColumnStretchFactor(0, 1);
        subLayout->setColumnStretchFactor(1, 0.01);
        subLayout->setRowStretchFactor(0, 1);
        subLayout->setRowStretchFactor(1, 0.01);

        QFont legendFont = font();
        legendFont.setPointSize(8);
        legend->setFont(legendFont);
        legend->setSelectedFont(legendFont);
        legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    }
    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
//    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

}

QPen TPPlot::newColorPen(int r, int g, int b, int width)
{
    QPen graphPen;
    graphPen.setColor(QColor(r, g, b));
    graphPen.setWidthF(width);
    return graphPen;
}
