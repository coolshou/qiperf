#ifndef TPCHART_H
#define TPCHART_H

#include <QObject>
#include <QWidget>

#include "qcustomplot.h"

//#include <QtCharts/QtCharts>
//#include <QtCharts/QChartView>
//#include <QtCharts/QChart>
//#include <QtCharts/QLineSeries>
//#include <QtCharts/QValueAxis>
#include <QList>
#include <QColor>
//QT_CHARTS_USE_NAMESPACE
#include "lib/axistag.h"

#define TEST_MOUSE_POS 0
#define TEST_FAKE_DATA 0

//class TPChart : public QCustomPlot
class TPChart : public QWidget
{
    Q_OBJECT
public:
    explicit TPChart(QWidget *parent = nullptr);
    ~TPChart();
//    enum DirColor{
//        UP=QColor(40, 110, 255) //blue
//           QColor(255, 110, 40) //red
//        DN=
//    };
public slots:
    void realtimeDataSlot();
    void onMouseMoveEvent(QMouseEvent *event);
private:
    void initLegend();
    void testDynamicData();

private:
    QCustomPlot *mPlot;
    QCPGraph *m_graph;
    QCPGraph *m_graph2;
    AxisTag *mTag1;
    AxisTag *mTag2;
    QTimer *dataTimer;
//    QChart *m_chart;
//    QList<QLineSeries> *m_lines;
//    QValueAxis *m_axisX;
//    QValueAxis *m_axisY;

};

#endif // TPCHART_H
