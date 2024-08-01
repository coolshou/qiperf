#ifndef TPPLOT_H
#define TPPLOT_H


#include <QObject>
#include <QTimer>
//#include <QCustomPlot>
#include "../lib/qcustomplot.h"
#include "comm.h"
//#include <QPrivateSignal>



class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit TPPlot(QWidget *parent = nullptr);
    void setStartTime(QDateTime startTime);
    void addTPData(QString idx, double xdata, double ydata, int lostrate);  //
    QCPGraph *getGraph(QString idx); // get QCPGraph by index
    QCPBars *getLostRateGraph(QString idx);
    void clear();

public slots:
    void onIperfTPdata(QString sInterval, QString idx, QString data, QString lostrate);  //

private slots:
    void realtimeDataSlot(QPrivateSignal sig);

private:
    void initCustomPlot();
    void addRandomGraph();
    QPen newColorPen(int r, int g, int b, int width);
    QDateTime m_starttime;
    QTimer dataTimer;
    QMap<QString, QCPGraph *> m_graphs; // throughput graphs
    QMap<QString, QCPBars *> m_lostgraphs; // lost rate graphs
    QScrollArea *legendScrollArea;
    QWidget *legendContainer;
    QVBoxLayout *legendLayout;
    int m_yAxisMaxDefault=10; // 10 Mbps
    int m_xAxisMaxDefault=30; // 30sec
};

#endif // TPPLOT_H
