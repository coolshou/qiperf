#ifndef TPPLOT_H
#define TPPLOT_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QMutex> // Required for QMutex
#include <QMutexLocker> // Recommended for scoped locking

#include "../lib/qcustomplot.h"
#include "comm.h"
#include "myqcpgraph.h"
#include "myqcpbars.h"
#include "tpdata.h"

class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    enum GroupWidth{
        Total=3,
        Pair=2,
        Direction=2,
        Comment=2,
        Iperf=1,
    };
    Q_ENUM(GroupWidth)

public:
    // explicit TPPlot(bool showgroup, QString sunit, QWidget *parent = nullptr);
    explicit TPPlot(int tpgroup, QString sunit, QWidget *parent = nullptr);
    void addTPData(QString refrowidx, double xdata, double ydata, double lostrate);  //
    void del(QString idx);
    // QCPGraph *getGraph(QString idx, int width=1); // get QCPGraph by index
    MyQCPGraph *getGraph(QString refrowidx, int width=GroupWidth::Iperf); // get QCPGraph by index
    MyQCPBars *getLostRateGraph(QString refrowidx);
    void clear();
    void setXRangeUpper(double upper);

public slots:
    void onIperfTPdata(QString sInterval,
                       QString refrowidx, QString data, QString lostrate,
                       QString grouptag);  //
    void onIperfTPdatas(QString refrow, QString sInterval, const QJsonArray &dataarray);

    void setStartTime(QDateTime startTime);
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                         QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates);
    void setInterval(int interval);
    void setShowGroup(bool bShow);
    void setTPUint(QString tpunit);
    void onVLegendScrollChanged(int value);
    void onDataAdded(double key, double value);
    void onDatasSetted(QSharedPointer<QCPGraphDataContainer> data);
    void onLostRateDataAdded(double key, double value);
    void onLostRateDatasSetted(QSharedPointer<QCPBarsDataContainer> data);
    void setTestStarted(bool start);
signals:
    void selectedTPitem(QString idx);
    void sigLegendCount(int count);

private slots:
    void selectionChanged();
    void onXAxisRangeChanged(const QCPRange &newRange);
    void doReplot();
private:
    void initCustomPlot();
    QPen newColorPen(int r, int g, int b, int width);
    void updateXAxisRange(double mintime, double maxtime);
    void updateYAxisRange(double minvalue, double maxvalue);
    QVector<QCPGraphData> convertQMapToQVector(const QMap<double, double>& map);
    QSharedPointer<QCPGraphDataContainer> sumGraphData(const QSharedPointer<QCPGraphDataContainer> &data1, const QSharedPointer<QCPGraphDataContainer>& data2);
    QSharedPointer<QCPBarsDataContainer> sumLostGraphData(const QSharedPointer<QCPBarsDataContainer> &data1, const QSharedPointer<QCPBarsDataContainer>& data2);
    void calculateLegendItems();
    QCPDataContainer<QCPGraphData>::const_iterator findKeyValue(const QCPDataContainer<QCPGraphData> &container, double key);
    QDateTime m_starttime;
    // QMap<QString, QCPGraph *> m_graphs; // throughput graphs
    QMap<QString, MyQCPGraph *> m_graphs; // throughput graphs
    QMap<QString, MyQCPBars *> m_lostgraphs; // lost rate graphs
    QMap<QString, QCPAbstractLegendItem *> m_legends; // throughput legends
    QMap<QString, QCPAbstractLegendItem *> m_lostratelegends; // lost rate legends
    int m_yAxisMaxDefault=10; // 10 Mbps
    int m_xAxisMaxDefault=30; // 30sec
    int m_interval;
    bool m_showgroup;
    int m_tpgrouptype;
    QString m_tpunit;
    // QCPLayer *m_TotalLayer;
    // QCPLayer *m_LostRateLayer;
    MyQCPGraph *mTotalGraph;  //store total graph
    QCPAbstractLegendItem *mTotalLegendItem;//store total graph legend
    QVector<QCPGraphData> mTotalGraphData;
    MyQCPBars *mTotalLostGraph;  //store total lost rate graph
    QCPAbstractLegendItem *mTotalLostLegendItem; //store total lost rate graph legend

    QMap<double, double> mTotalData;
    // Add a mutex as a member variable
    QMutex m_mutex; // Protects access to mTotalGraph and related plot state
    QTimer *m_replottimer;
    double m_maxX;
    double m_maxY;
    QString m_TPUint;
    double m_timeWindowThreshold;
    bool m_autoScrollXAxis;
    bool m_isTestStarted;
};

#endif // TPPLOT_H
