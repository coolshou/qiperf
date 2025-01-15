#ifndef TPPLOT_H
#define TPPLOT_H


#include <QObject>
#include <QTimer>
#include "../lib/qcustomplot.h"
// #include "comm.h"
//#include <QPrivateSignal>

class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit TPPlot(QWidget *parent = nullptr);
    void addTPData(QString idx, double xdata, double ydata, double lostrate);  //
    void del(QString idx);
    QCPGraph *getGraph(QString idx); // get QCPGraph by index
    QCPBars *getLostRateGraph(QString idx);
    void clear();

public slots:
    void onIperfTPdata(QString sInterval, QString idx, QString data, QString lostrate);  //
    void setStartTime(QDateTime startTime);
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                         QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates);
signals:
    void selectedTPitem(QString idx);

private slots:
    void selectionChanged();
private:
    void initCustomPlot();
    QPen newColorPen(int r, int g, int b, int width);
    QDateTime m_starttime;
    QMap<QString, QCPGraph *> m_graphs; // throughput graphs
    QMap<QString, QCPBars *> m_lostgraphs; // lost rate graphs
    int m_yAxisMaxDefault=10; // 10 Mbps
    int m_xAxisMaxDefault=30; // 30sec
};

#endif // TPPLOT_H
