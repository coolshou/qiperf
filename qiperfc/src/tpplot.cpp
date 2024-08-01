#include "tpplot.h"

TPPlot::TPPlot(QWidget *parent):QCustomPlot(parent)
{
    initCustomPlot();
}

void TPPlot::setStartTime(QDateTime startTime)
{
    m_starttime = startTime;

}

void TPPlot::onIperfTPdata(QString sInterval, QString idx, QString data, QString lostrate)
{
    double x = sInterval.toDouble();
    double y = data.toDouble();
    addTPData(idx, x, y, lostrate.toInt());
}

void TPPlot::addTPData(QString idx, double xdata, double ydata, int lostrate)
{
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
        g_lostrate->addData(xdata, lostrate);
    }
    this->replot();
}

QCPGraph *TPPlot::getGraph(QString idx)
{
    QPen graphPen;
    QCPGraph *g;
    if (!m_graphs.contains(idx)){
        g = this->addGraph(xAxis, yAxis);
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
    m_graphs.insert(idx,g);
    return g;
}

QCPBars *TPPlot::getLostRateGraph(QString idx)
{
    QPen graphPen;
    QCPGraph *g;
    QCPBars *g_lostrate;
    if (!m_lostgraphs.contains(idx)){
        if (m_graphs.contains(idx)){
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
    m_lostgraphs.insert(idx,g_lostrate);
    return g_lostrate;
}

void TPPlot::clear()
{
    this->clearGraphs();
    m_graphs.clear();
    for (auto it = m_lostgraphs.begin(); it != m_lostgraphs.end(); ++it) {
        this->removePlottable(it.value());
    }
    m_lostgraphs.clear();
    //axis reset
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);

    this->replot();
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
    this->legend->setVisible(true);
    if (0){//TODO: not good on layout
        // Add the QCustomPlot legend to the container
        QCPLayoutGrid *subLayout = new QCPLayoutGrid;
        //TODO: position the legend outside of the graph!!
        //
        this->plotLayout()->addElement(0, 1, subLayout);
        this->plotLayout()->setColumnStretchFactor(0, 1);
        this->plotLayout()->setColumnStretchFactor(1, 0.1); // col 1
        this->plotLayout()->setRowStretchFactor(0, 1); // row 0
    //    subLayout->addElement(0, 0, new QCPLayoutElement); // row 0
        subLayout->addElement(0, 0, this->legend); // row 0
        subLayout->addElement(1, 0, new QCPLayoutElement); // row 1
        subLayout->setRowStretchFactor(1, 0.001);
    //    this->plotLayout()->setRowStretchFactor(2, 0.001);

        QFont legendFont = font();
        legendFont.setPointSize(10);
        this->legend->setFont(legendFont);
        this->legend->setSelectedFont(legendFont);
        this->legend->setSelectableParts(QCPLegend::spItems); // legend box shall not be selectable, only legend items
    }
    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2, SLOT(setRange(QCPRange)));
//    connect(yAxis, SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));

}
void TPPlot::addRandomGraph()
{ // Test QCustomPlot
  int n = 50; // number of points in graph
    double xScale = (rand()/static_cast<double>(RAND_MAX) + 0.5)*2;
  double yScale = (rand()/static_cast<double>(RAND_MAX) + 0.5)*2;
    double xOffset = (rand()/static_cast<double>(RAND_MAX) - 0.5)*4;
  double yOffset = (rand()/static_cast<double>(RAND_MAX) - 0.5)*10;
    double r1 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  double r2 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
    double r3 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  double r4 = (rand()/static_cast<double>(RAND_MAX) - 0.5)*2;
  QVector<double> x(n), y(n);
  for (int i=0; i<n; i++)
  {
        x[i] = (i/static_cast<double>(n)-0.5)*10.0*xScale + xOffset;
    y[i] = (qSin(x[i]*r1*5)*qSin(qCos(x[i]*r2)*r4*3)+r3*qCos(qSin(x[i])*r4*2))*yScale + yOffset;
  }

  this->addGraph();
  this->graph()->setName(QString("New graph %1").arg(this->graphCount()-1));
  this->graph()->setData(x, y);
  this->graph()->setLineStyle(QCPGraph::lsLine);
//  this->graph()->setLineStyle((QCPGraph::LineStyle)(rand()%5+1));
//  if (rand()%100 > 50)
//    this->graph()->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)(rand()%14+1)));
  QPen graphPen;
//  graphPen.setColor(QColor(rand()%245+10, rand()%245+10, rand()%245+10));
//  graphPen.setWidthF(rand()/(double)RAND_MAX*2+1);
  graphPen = newColorPen(rand()%245+10, rand()%245+10, rand()%245+10, 1);
  this->graph()->setPen(graphPen);
  this->replot();
}

QPen TPPlot::newColorPen(int r, int g, int b, int width)
{
    QPen graphPen;
    graphPen.setColor(QColor(r, g, b));
    graphPen.setWidthF(width);
    return graphPen;
}

void TPPlot::realtimeDataSlot(QPrivateSignal sig)
{ //test live data
    Q_UNUSED(sig)
    static double time = 0;
    time += 1;
    double value = QRandomGenerator::global()->generate() % 1000; // Simulated data value
    double value2 = QRandomGenerator::global()->generate() % 1000;
    // Add the data point to the graph
    this->graph(0)->addData(time, value);
    this->graph(1)->addData(time, value2);

    xAxis->setRange(time, 120, Qt::AlignRight);
    this->replot();
}
