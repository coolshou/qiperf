#include "tpchart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QRandomGenerator>

#include <QDebug>

TPChart::TPChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_series(0),
    m_axisX(new QValueAxis()),
    m_axisY(new QValueAxis()),
    m_step(0),
    m_x(5),
    m_y(1)
{
//    setTitle("Throughput");
    setAnimationOptions(QChart::AllAnimations);
    legend()->setAlignment(Qt::AlignRight);
    QObject::connect(&m_timer, &QTimer::timeout, this, &TPChart::handleTimeout);
    m_timer.setInterval(1000);
    addAxis(m_axisX,Qt::AlignBottom);
    addAxis(m_axisY,Qt::AlignLeft);
    //for test
    add_series("1",Qt::red);
    add_series("2",Qt::green);

    m_axisX->setTickCount(5);
    m_axisX->setRange(0, 10);
    m_axisY->setRange(-5, 10);
    qDebug() << " start timer " << Qt::endl;
    m_timer.start();
}

TPChart::~TPChart()
{

}
void TPChart::add_series(QString idx, QColor pencolor){
    QSplineSeries *series = new QSplineSeries(this);
    m_dict_series[idx] = series;
    series->setName(idx);
    QPen pen(pencolor);
    pen.setWidth(1);
    series->setPen(pen);
    series->append(m_x, m_y);

    addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);
}

void TPChart::update_series_data(QString idx, qreal x, qreal y)
{
    if (m_dict_series.contains(idx)){
        QSplineSeries *series = m_dict_series.value(idx);
        series->append(x, y);
    }
}
void TPChart::handleTimeout()
{   //for test
    qDebug() << " handleTimeout " << Qt::endl;
    qreal x = plotArea().width() / m_axisX->tickCount();
    qreal y = (m_axisX->max() - m_axisX->min()) / m_axisX->tickCount();
    qDebug() << "x:" << QString::number(x) << ", y:" << QString::number(y) << Qt::endl;

    m_x += y;
    m_y = QRandomGenerator::global()->bounded(5) - 2.5;
    qreal y2 = QRandomGenerator::global()->bounded(5) - 1.5;

    update_series_data("1",m_x, m_y);
    update_series_data("2",m_x, y2);
//    m_series->append(m_x, m_y);
    scroll(x, 0);
    if (m_x == 100)
        m_timer.stop();
}
