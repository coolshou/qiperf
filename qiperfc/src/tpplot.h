#ifndef TPPLOT_H
#define TPPLOT_H

#include <QMap>
#include <QMutex>       // Required for QMutex
#include <QMutexLocker> // Recommended for scoped locking
#include <QObject>
#include <QTimer>

#include "../lib/qcustomplot.h"
#include "comm.h"
#include "myqcpbars.h"
#include "myqcpgraph.h"
#include "tpdata.h"
#include "tpgroup.h"

class TPPlot : public QCustomPlot {
  Q_OBJECT
public:
  enum GroupWidth {
    Total = 3,
    Pair = 2,
    Direction = 2,
    Comment = 2,
    Iperf = 1,
  };
  Q_ENUM(GroupWidth)

public:
  // explicit TPPlot(bool showgroup, QString sunit, QWidget *parent = nullptr);
  explicit TPPlot(int tpgroup, QString sunit, int xAxisMaxDefault = 60, QWidget *parent = nullptr);
  void addTPData(QString refrowidx, double xdata, double ydata, //double lostrate,
                 qint64 pktlost, qint64 pkttotal,
                 QString grouptag = ""); //
  void del(QString idx);
  MyQCPGraph *getGraph(QString refrowidx, int dir=-1,
                       int width = GroupWidth::Iperf); // get QCPGraph by index
  MyQCPBars *getLostRateGraph(QString refrowidx, int dir=-1);
  void clear();
  void setXRangeUpper(double upper);

  void clearLegendItems();
  void addThroughputLegendItems(bool bshowDetail, bool bshowDirection, bool bshowTotal, int startidx=0);
  void addLostRateLegendItems(bool bshowDetail, bool bshowDirection, bool bshowTotal, int startidx=0);

  public slots:
  void onIperfTPdata(QString sInterval, QString refrowidx, QString data,
                     qint64 pktlost, qint64 pkttotal,
                     // QString lostrate,
                     QString grouptag); //
  void onIperfTPdatas(QString refrow, QString sInterval,
                      const QJsonArray &dataarray);

  void setStartTime(QDateTime startTime);
  void onUpdateTPDatas(QString refrow, QVector<double> timedatas,
                       QVector<double> valuedatas, QVector<int> packetlosts,
                       QVector<int> packettotals, QVector<double> lostrates,
                       int direction);
  void setInterval(int interval);
  // void setShowGroup(bool bShow);
  void setTPGroupType(int grouptype);
  void setTPUint(QString tpunit);
  void onVLegendScrollChanged(int value);
  // void onDataAdded(double key, double value);
  void onDatasSetted(QSharedPointer<QCPGraphDataContainer> data, int dir);
  // void onLostRateDataAdded(double key, double value);
  void onLostRateDatasSetted(QSharedPointer<QCPBarsDataContainer> data);
  void setTestStarted(bool start);
  void calculateLegendItems();
  void onBeforeReplot();
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
  QVector<QCPGraphData> convertQMapToQVector(const QMap<double, double> &map);
  // QSharedPointer<QCPGraphDataContainer>
  // sumGraphData(const QSharedPointer<QCPGraphDataContainer> &data1,
  //              const QSharedPointer<QCPGraphDataContainer> &data2);
  QSharedPointer<QCPBarsDataContainer>
  sumLostGraphData(const QSharedPointer<QCPBarsDataContainer> &data1,
                   const QSharedPointer<QCPBarsDataContainer> &data2);

  QCPDataContainer<QCPGraphData>::const_iterator
  findKeyValue(const QCPDataContainer<QCPGraphData> &container, double key);
  QDateTime m_starttime;
  // QMap<QString, QCPGraph *> m_graphs; // throughput graphs
  QMap<QString, MyQCPGraph *> m_graphs;    // throughput graphs
  QMap<QString, MyQCPBars *> m_lostgraphs; // lost rate graphs
  QList<QString> m_legendKeys; // 用來維護插入順序
  QHash<QString, QCPAbstractLegendItem *> m_legends; // legends to store not visible legend item
  QList<QString> m_lostratelegendKeys; // 用來維護插入順序
  QHash<QString, QCPAbstractLegendItem *> m_lostratelegends; // lost rate legends
  int m_yAxisMaxDefault = 10;                               // 10 Mbps
  int m_xAxisMaxDefault;                               // default xAxis Max value, 60sec
  int m_interval;
  int m_tpgrouptype;
  QString m_tpunit;
  QCPLayoutElement *spacer;
  // QCPLayer *m_TotalLayer;
  // QCPLayer *m_LostRateLayer;
  MyQCPGraph *mTotalGraph;                           // store total graph
  QCPAbstractLegendItem *mTotalLegendItem = nullptr; // store total graph legend
  QVector<QCPGraphData> mTotalGraphData;
  MyQCPBars *mTotalLostGraph; // store total lost rate graph
  QCPAbstractLegendItem *mTotalLostLegendItem =
      nullptr; // store total lost rate graph legend
  QMap<double, double> mTotalData;
  // Direction - Tx
  MyQCPGraph *mDirTxGraph;                           // store Tx graph
  QCPAbstractLegendItem *mDirTxLegendItem = nullptr; // store tx graph legend
  QVector<QCPGraphData> mDirTxGraphData;
  MyQCPBars *mDirTxLostGraph; // store tx lost rate graph
  QCPAbstractLegendItem *mDirTxLostLegendItem =
      nullptr; // store tx lost rate graph legend
  QMap<double, double> mDirTxData;
  // Direction - Rx
  MyQCPGraph *mDirRxGraph;                           // store Rx graph
  QCPAbstractLegendItem *mDirRxLegendItem = nullptr; // store rx graph legend
  QVector<QCPGraphData> mDirRxGraphData;
  MyQCPBars *mDirRxLostGraph; // store rx lost rate graph
  QCPAbstractLegendItem *mDirRxLostLegendItem =
      nullptr; // store rx lost rate graph legend
  QMap<double, double> mDirRxData;
  QVector<QCPAbstractLegendItem*> m_savedLegendItems;

  // TODO Comment
  // Add a mutex as a member variable
  QMutex m_mutex; // Protects access to mTotalGraph and related plot state
  QTimer *m_replottimer;
  double m_maxX;
  double m_maxY;
  QString m_TPUint;
  double m_timeWindowThreshold;
  bool m_autoScrollXAxis;
  bool m_isTestStarted;
  void accumulateData(MyQCPGraph *targetGraph, const QSharedPointer<QCPGraphDataContainer> &newData);

  int m_maxLegendItemCount; // store max legend items can show in legend area

};

#endif // TPPLOT_H
