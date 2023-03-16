#include "tpchart.h"
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QVBoxLayout>
#include <QPen>
#include <QSize>

#include <QDebug>

//TPChart::TPChart(QWidget *parent):QCustomPlot(parent)
  TPChart::TPChart(QWidget *parent):QWidget(parent)
{
    mPlot = new QCustomPlot(this);
//    mPlot->setMinimumSize(QSize(500,500));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mPlot);
    initLegend();
//    mPlot->axisRect()->setupFullAxesBox(true);
//    mPlot->axisRect()->setAutoMargins(QCP::msLeft|QCP::msBottom|QCP::msRight|QCP::msTop);

    // zoom and drag only on horrizontal axis
    mPlot->axisRect()->setRangeZoomAxes(mPlot->xAxis,nullptr);
    mPlot->axisRect()->setRangeDragAxes(mPlot->xAxis,nullptr);
    ////設置曲線可拖曳 滾輪放大縮小 圖像可選擇
    mPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    //顯示鼠標點所在座標值
#if TEST_MOUSE_POS==1
    connect(mPlot,&QCustomPlot::mouseMove,this,&TPChart::onMouseMoveEvent);
#endif
    // configure bottom axis to show date instead of number:
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
//    dateTicker->setDateTimeFormat("HHmmss\nyyyyMMdd");
    timeTicker->setTimeFormat("%h:%m:%s");
    mPlot->xAxis->setTicker(timeTicker);

//    mPlot->xAxis->rescale();
    mPlot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(60);
//    mPlot->axisRect()->axis(QCPAxis::atTop, 0)->setPadding(10);
//    mPlot->xAxis->rescale();

    // set axis ranges to show all data:
//    double now = QDateTime::currentDateTimeUtc().toTime_t();
//    this->xAxis->setRange(now, now+24*3600*249); //Min/Max dynamic change
    //this->yAxis->setRange(0, 1000); // Max value may change!!
//    this->yAxis->setRange(-1.2, 1.2);

    //# TODO: how to init max size without using addGraph()??
    m_graph = mPlot->addGraph(); // add a graph, the chart will enlarge to fit widget
    m_graph->rescaleAxes(true);

    mPlot->xAxis->setLabel("Time");
    mPlot->yAxis->setLabel("Mbps");
    // make top and right axes visible but without ticks and labels:
    mPlot->xAxis2->setVisible(true);
    mPlot->yAxis2->setVisible(true);
    mPlot->xAxis2->setTicks(false);
    mPlot->yAxis2->setTicks(false);
    mPlot->xAxis2->setTickLabels(false);
    mPlot->yAxis2->setTickLabels(false);

#if TEST_FAKE_DATA==1
    testDynamicData();
#endif

}

void TPChart::realtimeDataSlot()
{
    // calculate and add a new data point to each graph:
    m_graph->addData(m_graph->dataCount(), qSin(m_graph->dataCount()/50.0)+qSin(m_graph->dataCount()/50.0/0.3843)*0.25);
    m_graph2->addData(m_graph2->dataCount(), qCos(m_graph2->dataCount()/50.0)+qSin(m_graph2->dataCount()/50.0/0.4364)*0.15);
    // make key axis range scroll with the data:
    mPlot->xAxis->rescale();
    m_graph->rescaleValueAxis(false, true);
    m_graph2->rescaleValueAxis(false, true);
    mPlot->xAxis->setRange(mPlot->xAxis->range().upper, 100, Qt::AlignRight);
    // update the vertical axis tag positions and texts to match the rightmost data point of the graphs:
    double graph1Value = m_graph->dataMainValue(m_graph->dataCount()-1);
    double graph2Value = m_graph2->dataMainValue(m_graph2->dataCount()-1);
    mTag1->updatePosition(graph1Value);
    mTag2->updatePosition(graph2Value);
    mTag1->setText(QString::number(graph1Value, 'f', 2));
    mTag2->setText(QString::number(graph2Value, 'f', 2));

    mPlot->replot();

}

void TPChart::onMouseMoveEvent(QMouseEvent *event)
{
    double x = event->pos().x();
    double y = event->pos().y();

    double x_ = mPlot->xAxis->pixelToCoord(x);
    double y_ = mPlot->yAxis->pixelToCoord(y);

    QString str = QString("x:%1,y:%2").arg(QString::number(x_,10,3), QString::number(y_,10,3));
    QToolTip::showText(cursor().pos(),str, mPlot);
}

void TPChart::initLegend()
{   //init Legend position

    // show legend with slightly transparent background brush:
//    mPlot->legend->setVisible(true);
    mPlot->legend->setBrush(QColor(192, 192, 192, 50));
    mPlot->legend->setBorderPen(QPen(Qt::DashDotLine));

    //legend position
    // 新增層
    mPlot->plotLayout()->insertColumn(1);//->insertRow(1);

    // 這個可以 F1 查看幫助
    mPlot->plotLayout()->addElement(0 , 1, mPlot->legend);
    mPlot->plotLayout()->setColumnStretchFactor(1, 0.01);
    mPlot->plotLayout()->setRowStretchFactor(0, 1);
    mPlot->legend->setVisible(false);
}

void TPChart::testDynamicData()
{
    connect(mPlot->yAxis2, SIGNAL(rangeChanged(QCPRange)), mPlot->yAxis, SLOT(setRange(QCPRange)));
    mPlot->axisRect()->addAxis(QCPAxis::atRight);
    mPlot->axisRect()->axis(QCPAxis::atRight, 0)->setPadding(30); // add some padding to have space for tags
    mPlot->axisRect()->axis(QCPAxis::atRight, 1)->setPadding(30); // add some padding to have space for tags

    m_graph = mPlot->addGraph(mPlot->xAxis, mPlot->axisRect()->axis(QCPAxis::atRight, 0));
    m_graph->setName("test1");
    m_graph->setPen(QPen(QColor(40, 110, 255)));//blue
    m_graph2 = mPlot->addGraph(mPlot->xAxis, mPlot->axisRect()->axis(QCPAxis::atRight, 1));
    m_graph2->setName("test2");
    m_graph2->setPen(QPen(QColor(255, 110, 40)));//red
    mTag1 = new AxisTag(m_graph->valueAxis());
    mTag1->setPen(m_graph->pen());
    mTag2 = new AxisTag(m_graph2->valueAxis());
    mTag2->setPen(m_graph2->pen());

    //    this->setParent(parent);
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    dataTimer = new QTimer(this);
    connect(dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    qDebug() << "start timer" << Qt::endl;
    dataTimer->start(40); // Interval 0 means to refresh as fast as possible

}

