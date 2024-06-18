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
    void addTPData(QString sInterval, QString idx, QString data);  //
    void addTPData(QString idx, double xdata, double ydata);  //
    QCPGraph *getGraph(QString idx); // get QCPGraph by index
    void clear();

private slots:
    void realtimeDataSlot(QPrivateSignal sig);
private:
    void initCustomPlote();
    void addRandomGraph();
    QPen newColorPen(int r, int g, int b, int width);
    QDateTime m_starttime;
    QTimer dataTimer;
    QMap<QString, QCPGraph *> m_graphs;
    QScrollArea *legendScrollArea;
    QWidget *legendContainer;
    QVBoxLayout *legendLayout;
};

#endif // TPPLOT_H
