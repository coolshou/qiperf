#ifndef TPPLOT_H
#define TPPLOT_H

#include <QObject>
#include <QTimer>
#include <QMap>

#include "../lib/qcustomplot.h"
#include "comm.h"
#include "myqcpgraph.h"

class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit TPPlot(bool showgroup, QString sunit, QWidget *parent = nullptr);
    void addTPData(QString idx, double xdata, double ydata, double lostrate);  //
    void del(QString idx);
    // QCPGraph *getGraph(QString idx, int width=1); // get QCPGraph by index
    MyQCPGraph *getGraph(QString idx, int width=1); // get QCPGraph by index
    QCPBars *getLostRateGraph(QString idx);
    void clear();
    void setXRangeUpper(double upper);

public slots:
    void onIperfTPdata(QString sInterval, QString idx, QString data, QString lostrate);  //
    void setStartTime(QDateTime startTime);
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                         QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates);
    void setInterval(int interval);
    void setShowGroup(bool bShow);
    void setTPUint(QString tpunit);
    void onVLegendScrollChanged(int value);
    void onDataAdded(double key, double value);
    void onDatasSetted(QSharedPointer<QCPGraphDataContainer> data);
signals:
    void selectedTPitem(QString idx);
    void sigLegendCount(int count);

private slots:
    void selectionChanged();
private:
    void initCustomPlot();
    QPen newColorPen(int r, int g, int b, int width);
    // void updateTotalGraph();
    // void updateTotalGraphData(double targetKey, double value);
    void updateYAxisRange(double minvalue, double maxvalue);
    QVector<QCPGraphData> convertQMapToQVector(const QMap<double, double>& map);
    QSharedPointer<QCPGraphDataContainer> sumGraphData(const QSharedPointer<QCPGraphDataContainer> &data1, const QSharedPointer<QCPGraphDataContainer>& data2);
    void calculateLegendItems();
    QCPDataContainer<QCPGraphData>::const_iterator findKeyValue(const QCPDataContainer<QCPGraphData> &container, double key);
    QDateTime m_starttime;
    // QMap<QString, QCPGraph *> m_graphs; // throughput graphs
    QMap<QString, MyQCPGraph *> m_graphs; // throughput graphs
    QMap<QString, QCPBars *> m_lostgraphs; // lost rate graphs
    QMap<QString, QCPAbstractLegendItem *> m_legends; // throughput legends
    QMap<QString, QCPAbstractLegendItem *> m_lostratelegends; // lost rate legends
    int m_yAxisMaxDefault=10; // 10 Mbps
    int m_xAxisMaxDefault=30; // 30sec
    int m_interval;
    bool m_showgroup;
    QString m_tpunit;
    MyQCPGraph *mTotalGraph;
    QCPAbstractLegendItem *mTotalLegendItem;
    QVector<QCPGraphData> mTotalGraphData;
    QMap<double, double> mTotalData;
};

#endif // TPPLOT_H
