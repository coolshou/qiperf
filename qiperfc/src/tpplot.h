#ifndef TPPLOT_H
#define TPPLOT_H


#include <QObject>
#include <QTimer>
//#include <QCustomPlot>
#include "lib/qcustomplot.h"
#include "comm.h"
//#include <QPrivateSignal>



class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit TPPlot(QWidget *parent = nullptr);
    void setStartTime(QDateTime startTime);
    void addTPDatas(QString sInterval, QString idx, QString datas); // time sInterval, parallel number , throughput data list
    void addTPData(QString idx, double xdata, double ydata);  //
    QCPGraph *getGraph(QString idx); // get QCPGraph by index
    void clear();

public slots:
    void onIperfTPdata(QString sInterval, QString idx, QString data);  //

private slots:
    void realtimeDataSlot(QPrivateSignal sig);

private:
    void initCustomPlot();
    void addRandomGraph();
    QPen newColorPen(int r, int g, int b, int width);
    QDateTime m_starttime;
    QTimer dataTimer;
    QMap<QString, QCPGraph *> m_graphs;
    QScrollArea *legendScrollArea;
    QWidget *legendContainer;
    QVBoxLayout *legendLayout;
    int m_yAxisMaxDefault=100; // 100 Mbps
    int m_xAxisMaxDefault=30; // 30sec
};

#endif // TPPLOT_H
