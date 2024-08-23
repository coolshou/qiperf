#include "pingplot.h"

PingPlot::PingPlot(QWidget *parent):QCustomPlot(parent)
{
    initCustomPlot();
}

void PingPlot::initCustomPlot()
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
    yAxis->setLabel("Response(Sec)");
    yAxis2->setLabel("Lost Rate(%)");
    yAxis2->setTickLabels(true);
    //set axis range
    // TODO: update range by throughput/time
    xAxis->setRange(0, m_xAxisMaxDefault);
    yAxis->setRange(0, m_yAxisMaxDefault);
    yAxis2->setRange(0, 100); // %
    // legend
    legend->setVisible(true);
}
