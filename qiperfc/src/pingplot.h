#ifndef PINGPLOT_H
#define PINGPLOT_H

//#include <QCustomPlot>
#include "../lib/qcustomplot.h"
#include <QObject>

class PingPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit PingPlot(QWidget *parent = nullptr);
private:
    void initCustomPlot();
    int m_yAxisMaxDefault=10; // 10 Mbps
    int m_xAxisMaxDefault=30; // 30sec

};

#endif // PINGPLOT_H
