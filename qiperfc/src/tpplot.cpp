#include "tpplot.h"

#include <QScreen>
#include <numeric>
#include <algorithm> // For std::find_if
#include <iterator>  // For std::make_reverse_iterator

#include "tpmgrdata.h"

TPPlot::TPPlot(int tpgroup, QString sunit, QWidget *parent)
    : QCustomPlot(parent), m_tpgrouptype(tpgroup)
{
  m_isTestStarted = false;
  m_maxX = 30;
  m_maxY = m_yAxisMaxDefault;
  // TODO: when total test time smaller then this, need update?
  m_timeWindowThreshold = 10 ; //30.0;
  m_autoScrollXAxis = true;
  QScreen *screen = QGuiApplication::primaryScreen();
  bool b4K = false;
  bool scale = false; //Window scale
  if (screen)
  {
    int logicalWidth = screen->geometry().width();
    qreal dpr = screen->devicePixelRatio();
    if (dpr > 1) {//NOTE: when scale > 100%, it will cause graph size strange on OpenGL enable!!
        scale = true;
    }
    int physicalWidth = qRound(logicalWidth * dpr);

    qDebug() << "Current logicalWidth:" << logicalWidth
             << " physicalWidth:" << physicalWidth;
    if (physicalWidth >= 3840)
    {
      b4K = true;
    }
  }
  setOpenGl(!b4K & scale);
  setNoAntialiasingOnDrag(true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_interval = 1;
  setTPUint(sunit);
  qDebug() << "parent geometry:" << parent->geometry();
  initCustomPlot();
  QCPLayer *mainlayer = layer(LAYER_MAIN);
  if (!addLayer(LAYER_TOTAL, mainlayer, limAbove))
  {
    qDebug() << "addLayer " << LAYER_TOTAL << " Fail";
  }
  if (!addLayer(LAYER_TOTALOSTRATE, mainlayer, limBelow))
  {
    qDebug() << "addLayer " << LAYER_TOTALOSTRATE << " Fail";
  }
  if (!addLayer(LAYER_LOSTRATE, layer(LAYER_TOTALOSTRATE), limBelow))
  {
    qDebug() << "addLayer " << LAYER_LOSTRATE << " Fail";
  }
  if (!addLayer(LAYER_DIR, layer(LAYER_LOSTRATE), limBelow))
  {
    qDebug() << "addLayer " << LAYER_DIR << " Fail";
  }
  if (!addLayer(LAYER_DIRLOSTRATE, layer(LAYER_DIR), limBelow))
  {
    qDebug() << "addLayer " << LAYER_DIRLOSTRATE << " Fail";
  }

  getGraph(GRAPH_TOTAL, -1, GroupWidth::Total);
  getGraph(GRAPH_TX, 0, GroupWidth::Direction);
  getGraph(GRAPH_RX, 1, GroupWidth::Direction);
  qDebug() << "mTotalGraph: " << mTotalGraph << ", mDirTxGraph: " << mDirTxGraph
           << ", mDirRxGraph: " << mDirRxGraph;
  qDebug() << "mTotalLegendItem: " << mTotalLegendItem
           << ", mTotalLostLegendItem: " << mTotalLostLegendItem
           << ", mDirTxLegendItem: " << mDirTxLegendItem
           << ", mDirTxLostLegendItem: " << mDirTxLostLegendItem
           << ", mDirRxLegendItem: " << mDirRxLegendItem
           << ", mDirRxLostLegendItem: " << mDirRxLostLegendItem;
  setTPGroupType(m_tpgrouptype);
  m_replottimer = new QTimer(this);
  connect(m_replottimer, &QTimer::timeout, this, &TPPlot::doReplot);
  // clear(); // this will let plot layout looks strange!!
  // setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // TODO: init plot chart size not good to fit parent's rect

  // m_replottimer->start(500);//0.5 sec replot
  // m_replottimer->start(33);// 大約 30 FPS
  m_replottimer->start(100);
}

void TPPlot::setStartTime(QDateTime startTime) { m_starttime = startTime; }

void TPPlot::onUpdateTPDatas(QString refrow, QVector<double> timedatas,
                             QVector<double> valuedatas,
                             QVector<int> packetlosts,
                             QVector<int> packettotals,
                             QVector<double> lostrates,
                             int direction)
{
  if (timedatas.isEmpty() || valuedatas.isEmpty())
    return;

  qDebug() << "onUpdateTPDatas:" << refrow << " times:" << timedatas;
        //   << " values: " << valuedatas;
  MyQCPGraph *myGraph = getGraph(refrow, direction);

  double minT =
      *std::min_element(timedatas.begin(), timedatas.end()); // x: min time
  double maxT =
      *std::max_element(timedatas.begin(), timedatas.end()); // x: max time
  // qDebug() << "min:" << QString::number(minT) << " Max:" << QString::number(maxT);
  // updateXAxisRange(minT, maxT);

  double minV =
      *std::min_element(valuedatas.begin(), valuedatas.end()); // y: min value
  double maxV =
      *std::max_element(valuedatas.begin(), valuedatas.end()); // y: max value
  updateYAxisRange(minV, maxV);
  // qDebug() << "refrow:" << refrow << " timedatas: " << timedatas;
  myGraph->setData(timedatas, valuedatas);
  // Calculate the sum
  //   Q_UNUSED(packetlosts)
  int sum = std::accumulate(packettotals.begin(), packettotals.end(), 0);
  if (sum > 0)
  {
    // int lost = std::accumulate(packetlosts.begin(), packetlosts.end(), 0);
    MyQCPBars *g_lostrate = getLostRateGraph(refrow, direction);
    // lostrate
    Q_UNUSED(lostrates)
    g_lostrate->setData(timedatas, packetlosts, packettotals);
  }
  // this->replot();//20260122 tmp remove, use QTimer()
}

void TPPlot::setInterval(int interval) { m_interval = interval; }

void TPPlot::setTPGroupType(int grouptype)
{
  m_tpgrouptype = grouptype;
  bool bshowTotal = (m_tpgrouptype == TPGroup::GroupMode::Total);
  bool bshowDetail = (m_tpgrouptype == TPGroup::GroupMode::Detail);
  bool bshowDirection = (m_tpgrouptype == TPGroup::GroupMode::Direction);
  // bool bshowComment=(m_tpgrouptype == TPGroup::GroupMode::Comment);

  auto shouldShow = [&](QCPAbstractPlottable *item) -> bool
  {
    if (item == mTotalGraph || item == mTotalLostGraph)
    {
      return bshowTotal;
    }
    if (item == mDirTxGraph || item == mDirRxGraph ||
        item == mDirTxLostGraph || item == mDirRxLostGraph)
    {
      return bshowDirection;
    }
    return bshowDetail;
  };

  // === 统一设置 visible ===
  for (auto *g : m_graphs)
    g->setVisible(shouldShow(g));
  for (auto *b : m_lostgraphs)
    b->setVisible(shouldShow(b));

  // store all item in legend
  // 步驟 1：把目前 legend 裡面的item抽離（不 delete 記憶體）
  for (int i = legend->itemCount() - 1; i >= 0; --i)
  {
    QCPAbstractLegendItem *itm = legend->item(i);
    if (itm)
    {
      legend->take(itm);
      itm->setVisible(false);
    }
  }

  // 步驟 2：依據目前的模式，重新把符合條件的項目「依序」加回 legend 中
  // 2.1 處理 Throughput legends 圖例
  QMap<QString, QCPAbstractLegendItem *>::const_iterator legenditerator =
      m_legends.constBegin();
  while (legenditerator != m_legends.constEnd())
  {
    QCPAbstractLegendItem *item = legenditerator.value();
    bool shouldShow = false;
    if (bshowTotal)
    {
      if (item == mTotalLegendItem)
        shouldShow = true;
    }
    else if (bshowDirection)
    {
      if (item == mDirTxLegendItem || item == mDirRxLegendItem)
        shouldShow = true;
    }
    else if (bshowDetail)
    {
      // 排除 Total 與 Direction 的圖例，只顯示 Detail 圖例
      if (item != mTotalLegendItem && item != mDirTxLegendItem &&
          item != mDirRxLegendItem)
      {
        shouldShow = true;
      }
    }
    if (shouldShow)
    {
      legend->addElement(legend->elementCount(), 0, item);
      item->setVisible(true);
    }
    ++legenditerator;
  }
  // 2.2 處理 Lost Rate legends 圖例
  QMap<QString, QCPAbstractLegendItem *>::const_iterator lostlegenditerator =
      m_lostratelegends.constBegin();
  while (lostlegenditerator != m_lostratelegends.constEnd())
  {
    QCPAbstractLegendItem *item = lostlegenditerator.value();
    bool shouldShow = false;
    if (bshowTotal)
    {
      if (item == mTotalLostLegendItem)
        shouldShow = true;
    }
    else if (bshowDirection)
    {
      if (item == mDirTxLostLegendItem || item == mDirRxLostLegendItem)
        shouldShow = true;
    }
    else if (bshowDetail)
    {
      if (item != mTotalLostLegendItem && item != mDirTxLostLegendItem &&
          item != mDirRxLostLegendItem)
      {
        shouldShow = true;
      }
    }
    if (shouldShow)
    {
      legend->addElement(legend->elementCount(), 0, item);
      item->setVisible(true);
    }
    ++lostlegenditerator;
  }
  // 簡化排版，移除因為拿掉項目留下的空列
  legend->simplify();

  replot();
}

void TPPlot::setTPUint(QString tpunit)
{
  // conver iperf's unit to display string
  m_TPUint = tpunit;
  QString sunit = "Mbps";
  if (tpunit.contains("Kbits"))
  {
    sunit = "Kbps";
  }
  else if (tpunit.contains("Mbits"))
  {
    sunit = "Mbps";
  }
  else if (tpunit.contains("Gbits"))
  {
    sunit = "Gbps";
  }
  else if (tpunit.contains("Tbits"))
  {
    sunit = "Tbps";
  }
  else if (tpunit.contains("KBytes"))
  {
    sunit = "KB/s";
  }
  else if (tpunit.contains("MBytes"))
  {
    sunit = "MB/s";
  }
  else if (tpunit.contains("GBytes"))
  {
    sunit = "GB/s";
  }
  else if (tpunit.contains("TBytes"))
  {
    sunit = "TB/s";
  }
  yAxis->setLabel(sunit);
  replot();
}

void TPPlot::onVLegendScrollChanged(int value)
{
  qDebug() << "onVLegendScrollChanged:" << QString::number(value);
  if (m_tpgrouptype == TPGroup::GroupMode::Detail)
  {
    for (int i = 0; i < legend->itemCount(); ++i)
    {
      QCPAbstractLegendItem *item = legend->item(i);
      item->setVisible(i >= value && i < value + 10); // Show 10 items at a time
    }
    replot();
  }
}

// void TPPlot::onDataAdded(double key, double value) {
//   if (m_tpgrouptype == TPGroup::GroupMode::Total) {
//     // try calc Total Graph value form each Graphs
//     if (mTotalGraph) {
//       // add all value to Total graph's value

//       double orgvalue = 0;
//       // TODO : last record will be wrong!!??
//       QMutexLocker<QMutex> locker(&m_mutex); // Locks m_mutex, not work
//       int rc = mTotalGraph->getValue(key, orgvalue);
//       qDebug() << "key:" << QString::number(key)
//                << " orgvalue:" << QString::number(orgvalue)
//                << " new value:" << QString::number(value);
//       if (rc > -1) {
//         // sum up orgvalue & new value
//         double sumvalue = orgvalue + value;
//         updateYAxisRange(0, sumvalue);
//         qDebug() << "key:" << QString::number(key)
//                  << " sumvalue:" << QString::number(sumvalue);
//         mTotalGraph->updateValue(key, sumvalue);
//         mTotalGraph->rescaleAxes(true);
//       } else {
//         qDebug() << (static_cast<MyQCPGraph *>(sender()))->name() << " (" << key
//                  << ")mTotalGraph Key not found";
//         mTotalGraph->addData(key, value);
//       }
//     }
//   }
//   if (m_tpgrouptype == TPGroup::GroupMode::Direction) {
//     // TODO: direction
//     if (mDirTxGraph) {
//     }
//     if (mDirRxGraph) {
//     }
//   }
//   if (m_tpgrouptype == TPGroup::GroupMode::Comment) {
//     // TODO: Comment
//   }
// }
void TPPlot::accumulateData(MyQCPGraph* targetGraph, const QSharedPointer<QCPGraphDataContainer>& newData) {
    if (!targetGraph || !newData) return;

    auto targetData = targetGraph->data();
    // Loop through new data (can also use rbegin/rend here if newData needs to be read backwards)
    for (auto it = newData->constBegin(); it != newData->constEnd(); ++it) {
        // Create reverse iterators wrapped around regular forward iterators
        auto revBegin = std::make_reverse_iterator(targetData->end());
        auto revEnd   = std::make_reverse_iterator(targetData->begin());

        auto rIt = std::find_if(revBegin, revEnd, [it](const QCPGraphData& point) {
            return point.key <= it->key; // Stop as soon as target key range is passed
        });

        if (rIt != revEnd && rIt->key == it->key) {
            // Found exact match: update in-place
            rIt->value += it->value;
        } else {
            // Key not found: append/add point
            targetGraph->addData(it->key, it->value);
        }
    }
}

void TPPlot::onDatasSetted(QSharedPointer<QCPGraphDataContainer> data, int dir)
{
  // combine two data in to Total/Tx/Rx graph
  // dir: 0 Tx, 1: Rx
  QMutexLocker locker(&m_mutex); // Locks m_mutex

  if (mTotalGraph)
  {
    accumulateData(mTotalGraph, data);
    mTotalGraph->rescaleAxes(true);
  }
  qDebug() << "onDatasSetted dir:" << dir;
  if (dir == 0)
  { // Tx
    if (mDirTxGraph)
    {
        accumulateData(mDirTxGraph, data);
    }
    else
    {
      qDebug() << "No mDirTxGraph";
    }
  }
  else if (dir == 1)
  { // Rx
    if (mDirRxGraph)
    {
        accumulateData(mDirRxGraph, data);
    }
    else
    {
      qDebug() << "No mDirRxGraph";
    }
  }
  else
  {
    qDebug() << "onDatasSetted no more calc on dir=" << dir;
  }
}

// void TPPlot::onLostRateDataAdded(double key, double value)
// {
//   // LostRate is calc with lost/total packet
//   //
//   if (m_tpgrouptype == TPGroup::GroupMode::Total)
//   {
//     if (mTotalLostGraph)
//     {
//       // add all value to Total graph's value
//       double orgvalue = -1;
//       QMutexLocker locker(&m_mutex); // Locks m_mutex
//       int rc = mTotalLostGraph->getValue(key, orgvalue);
//       if (rc > -1)
//       {
//         double sumvalue = orgvalue + value;
//         // updateYAxisRange(0, sumvalue);
//         mTotalLostGraph->updateValue(key, sumvalue);
//         mTotalLostGraph->rescaleAxes(true);
//       }
//       else
//       {
//         qDebug() << (static_cast<MyQCPBars *>(sender()))->name() << " (" << key
//                  << ") mTotalLostGraph Key not found";
//         mTotalLostGraph->addData(key, value);
//       }
//     }
//   }
//   // TODO: Direction
// }

void TPPlot::onLostRateDatasSetted(QSharedPointer<QCPBarsDataContainer> data)
{
  QMutexLocker locker(&m_mutex); // Locks m_mutex
  if (mTotalLostGraph)
  {
    QSharedPointer<QCPBarsDataContainer> data1 = mTotalLostGraph->data();
    QSharedPointer<QCPBarsDataContainer> data2 = data;
    QSharedPointer<QCPBarsDataContainer> sumdata;
    sumdata = sumLostGraphData(data1, data2);
    mTotalLostGraph->setData(sumdata);
    mTotalLostGraph->rescaleAxes(true); // TODO: not good to show y Max value
    if (mTotalLostLegendItem)
    {
      mTotalLostLegendItem->setVisible(true);
    }
  }
  else
  {
    qDebug() << "onLostRateDatasSetted: ERROR does not have mTotalLostGraph";
  }
}

void TPPlot::setTestStarted(bool start) {
    m_isTestStarted = start;
    if (!m_isTestStarted){
        //throughput running had stop
        if (graphCount()) {
            qDebug() << "graphCount:" << QString::number(graphCount())
                     << " ,xAxis->range():" << xAxis->range();
            // xAxis->setRangeUpper();
            xAxis->rescale(true);
            yAxis->rescale(true);
            // rescaleAxes(true); // all Axis will fit
            replot();
        }
    }
}

void TPPlot::selectionChanged()
{
  /* synchronize the selection of the graphs with the selection state of the
respective legend item belonging to that graph. So the user can select a graph
by either clicking on the graph itself or on its legend item.*/
  // synchronize selection of graphs with selection of corresponding legend
  // items:
  for (int i = 0; i < this->graphCount(); i++)
  {
    QCPGraph *graph = this->graph(i);
    QCPPlottableLegendItem *item = this->legend->itemWithPlottable(graph);
    if (((item && item->selected()) or graph->selected()))
    {
      if (item)
      {
        item->setSelected(true);
      }
      if (graph)
      {
        graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
      }
      emit selectedTPitem(graph->name());
    }
    // QCoreApplication::processEvents(QEventLoop::AllEvents);
  }
}

void TPPlot::onXAxisRangeChanged(const QCPRange &newRange)
{
  // TODO: this will trigger on setting XAxis range by program?

  if (newRange.upper < m_maxX - 0.5)
  {
    // 使用者正在查看過去的數據，我們應該停止「自動捲動」
    m_autoScrollXAxis = false;
  }
  else
  {
    // 使用者拉回到了最右側，恢復自動捲動,
    m_autoScrollXAxis = true & m_isTestStarted;
  }
}

void TPPlot::doReplot()
{
  // qDebug() << "TPPlot size        :" << size();
  // qDebug() << "TPPlot geometry    :" << geometry();
  // qDebug() << "viewport           :" << viewport();
  // qDebug() << "axisRect outerRect :" << axisRect()->outerRect();

  // qDebug() << "plotLayout outer:" << plotLayout()->outerRect();
  // qDebug() << "plotLayout rect :" << plotLayout()->rect();

  // qDebug() << "axisRect minimum:" << axisRect()->minimumOuterSizeHint();
  // qDebug() << "legend minimum :" << legend->minimumOuterSizeHint();

  // qDebug() << "legend rect:" << legend->rect();
  // qDebug() << "legend outer:" << legend->outerRect();

  // qDebug() << "plotLayout rowCount =" << plotLayout()->rowCount();
  // qDebug() << "plotLayout columnCount =" << plotLayout()->columnCount();
  // qDebug() << legend->outerRect();

  // 保護機制：確保 Mutex 鎖定，因為我們在讀取可能被 addTPData 修改的變數
  QMutexLocker locker(&m_mutex);
  // 更新 Y 軸：加上一點緩衝空間 (例如 1.1 倍)，視覺上比較舒服
  // if (m_maxY > 0) {
  //     this->yAxis->setRange(0, m_maxY * 1.1);
  // }
  updateYAxisRange(0, m_maxY);

  // 更新 X 軸：典型的「滾動視窗」效果
  // 顯示最新的 m_interval 長度的區間
  // double windowSize = 60.0; // 顯示最近 60 秒
  // this->xAxis->setRange(m_maxX, windowSize, Qt::AlignRight);
  // int minx = 0;
  // if (m_maxX > 60){
  //     minx = m_maxX - 60;
  // }
  // updateXAxisRange(minx, m_maxX);
  if (m_autoScrollXAxis)
  {
    // updateXAxisRange(0, m_maxX);
  }

  // this->replot(QCustomPlot::rpQueuedReplot);
  this->replot(QCustomPlot::rpImmediateRefresh);

  // this->rpQueuedReplot();
}

void TPPlot::onIperfTPdata(QString sInterval, QString refrowidx, QString data,
                           qint64 pktlost, qint64 pkttotal, QString grouptag)
                           //QString lostrate,
{
  // double x = sInterval.toDouble();
  int x =
      static_cast<int>(sInterval.toDouble()); // ignore .0x Difference of xdata
  double y = round(data.toDouble()*100)/100;

  // qDebug() << "[TPPlot::onIperfTPdata]:" << refrowidx
  //          << " sInterval:" << sInterval << " x:" << QString::number(x)
  //          << " TP:" << QString::number(y)
  //          << " grouptag:" << grouptag;
  // addTPData(refrowidx, x, y, lostrate.toDouble(), grouptag);
  addTPData(refrowidx, x, y, pktlost, pkttotal, grouptag);
}

void TPPlot::onIperfTPdatas(QString refrow, QString sInterval,
                            const QJsonArray &dataarray)
{
  // add data by dataarray
  QString dir;
  QString idx;
  bool isAvg = false;
  QString value = "";
  QString unit = "";
  // QString slost_rate = "0";
  int iInterval = static_cast<int>(sInterval.toDouble());
  // double lost_rate = 0;
  double sumvalue = 0.0;
  double sumTxvalue = 0.0;
  double sumRxvalue = 0.0;
  // double sumlostrate = 0.0;
  qint64 sumpktlost = 0;
  qint64 sumpktTotal = 0;
  qint64 txpktlost = 0;
  qint64 txpktTotal = 0;
  // double txlostrate = 0.0;
  qint64 rxpktlost = 0;
  qint64 rxpktTotal = 0;
  // double rxlostrate = 0.0;
  qint64 pkt_lost = 0;
  qint64 pkt_total = 0;
  for (QJsonArray::const_iterator it = dataarray.constBegin();
       it != dataarray.constEnd(); ++it)
  {
    QJsonObject jObj = it->toObject();
    idx = jObj.value("idx").toString();
    isAvg = jObj.value("AVG").toBool();
    value = jObj.value("value").toString();
    if (!isAvg)
    {
      if (!jObj.value("dir").isUndefined())
      {
        dir = jObj.value("dir").toString();
      }
      unit = jObj.value("unit").toString();
      // sumvalue = round((sumvalue + value.toDouble())*100)/100;
      sumvalue = (sumvalue + value.toDouble());
      if (dir.contains(GRAPH_TX, Qt::CaseInsensitive))
      {
        sumTxvalue = round((sumTxvalue + value.toDouble())*100)/100;
      }
      if (dir.contains(GRAPH_RX, Qt::CaseInsensitive))
      {
        sumRxvalue = round((sumRxvalue + value.toDouble())*100)/100;
      }
      // if (QString::compare(unit, m_TPUint, Qt::CaseInsensitive) !=0){
      //     qDebug() << "//TODO: base on unit, convert the value to correct
      //     value"
      //              << " display unit:" << m_TPUint << " tp data unit:" <<
      //              unit;
      // }
      // packet lost rate
      pkt_lost = jObj.value("packet_lost").toString().toInt();
      pkt_total = jObj.value("packet_total").toString().toInt();
      if (pkt_lost > 0){
          sumpktlost = sumpktlost + pkt_lost;
          if (dir.contains(GRAPH_TX, Qt::CaseInsensitive)){
              txpktlost = txpktlost + pkt_lost;
          }
          if (dir.contains(GRAPH_RX, Qt::CaseInsensitive)){
              rxpktlost = rxpktlost + pkt_lost;
          }
      }
      if (pkt_total > 0){
          sumpktTotal = sumpktTotal + pkt_total;
          if (dir.contains(GRAPH_TX, Qt::CaseInsensitive)){
              txpktTotal = txpktTotal + pkt_total;
          }
          if (dir.contains(GRAPH_RX, Qt::CaseInsensitive)){
              rxpktTotal = rxpktTotal + pkt_total;
          }
      }
      onIperfTPdata(sInterval, refrow + "_" + idx, value, pkt_lost, pkt_total, dir);
    }
  }

  // qDebug() << "add:" << GRAPH_TOTAL << ", iInterval:" << QString::number(iInterval)
  //          << ",sumvalue:" << QString::number(sumvalue)
  //          << ",sumpktlost:" << QString::number(sumpktlost)
  //          << ",sumpktTotal:" << QString::number(sumpktTotal)
  //          << ",dir:" << dir;
  addTPData(GRAPH_TOTAL, iInterval, sumvalue, sumpktlost, sumpktTotal, dir);
  if (dir.contains(GRAPH_TX, Qt::CaseInsensitive))
  {
    addTPData(GRAPH_TX, iInterval, sumTxvalue, txpktlost, txpktTotal, dir);
  }
  if (dir.contains(GRAPH_RX, Qt::CaseInsensitive))
  {
    addTPData(GRAPH_RX, iInterval, sumRxvalue, rxpktlost, rxpktTotal, dir);
  }

  // let xAxis range in m_timeWindowThreshold, scroll when xdata >
  // m_timeWindowThreshold
  if (iInterval < m_timeWindowThreshold)
  {
      xAxis->setRange(0, m_timeWindowThreshold);
  }
  else
  {
      xAxis->setRange(iInterval, m_timeWindowThreshold, Qt::AlignRight);
  }

}

void TPPlot::addTPData(QString refrowidx, double xdata, double ydata,
                       qint64 pktlost, qint64 pkttotal, QString grouptag)
                       //double lostrate,
{
  QMutexLocker locker(&m_mutex); // Locks m_mutex,
  double sumydata = ydata;
  int dir = -1;
  // qDebug() << "grouptag:" << grouptag;
  if (grouptag.contains(TPDIRTx, Qt::CaseSensitive))
  {
    dir = 0;
  }
  if (grouptag.contains(TPDIRRx, Qt::CaseSensitive))
  {
    dir = 1;
  }
  MyQCPGraph *myGraph = getGraph(refrowidx, dir);

  // bool isTotal = refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive);
  if ((refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)) ||
      (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive)) ||
      (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive)))
  {
    // Total/Tx/Rx sum Throughput graph
    double oldvalue = 0.0;
    // qDebug() << refrowidx << ",xdata:" << QString::number(xdata)
    //          << ", count:" << myGraph->dataCount();
    if (myGraph->getValue(xdata, oldvalue) == -1)
    {
      myGraph->addData(xdata, ydata);
    }
    else
    {
      // qDebug() << xdata << ",found old value:" << QString::number(oldvalue);
      sumydata = sumydata + oldvalue;
      // qDebug() << xdata << ",new value:" << QString::number(sumydata);
      myGraph->updateValue(xdata, sumydata);
    }
  }
  else
  {
    // detail
    myGraph->addData(xdata, ydata);
  }

  // enlarge/shrink y range
  m_maxX = xdata + m_interval;
  if (sumydata > m_maxY)
  {
    m_maxY = sumydata;
  }
  // TODO: lost rate
  if ((pktlost > 0) && (pkttotal > 0)){
      // double lostrate = (static_cast<double>(pktlost)/pkttotal)*100.0;
      // qDebug() << refrowidx << " ,pktlost:" << pktlost << ",pkttotal:" << pkttotal
      //          << " ,lostrate:" << lostrate;
      MyQCPBars *g_lostrate = getLostRateGraph(refrowidx, dir);
      if ((refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive)) ||
          (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive)) ||
          (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive)))
      {
          PacketBarData olddata;
          if (!g_lostrate->getPacketData(xdata, olddata))
          {  //not found old value
              g_lostrate->addPacketData(xdata, pktlost, pkttotal);
          }
          else
          {
              int ipkt = olddata.pktlost + pktlost;
              int ipktall = olddata.pkttotal + pkttotal;
              // qDebug() << "TODO: lostrate can not sum directly, oldvalue:" << olddata.value
              //          << "sum pktlost:" << ipkt << " ,sum pkttotal:" << ipktall;
              g_lostrate->addPacketData(xdata, ipkt, ipktall);
          }
      }
      else
      {
          // detail
          g_lostrate->addPacketData(xdata, pktlost, pkttotal);
      }
  }
}

void TPPlot::del(QString idx)
{
  QMutexLocker locker(&m_mutex); // Locks m_mutex,
  if (m_lostgraphs.contains(idx))
  {
    QCPBars *b = m_lostgraphs.take(idx);
    if (removePlottable(b))
    {
      // TODO: why actually del but return false?
      //                qDebug() << "removePlottable QCPBars fail: " << idx;
    }
  }
  else
  {
    qDebug() << "m_lostgraphs not exist:" << idx;
  }
  if (m_graphs.contains(idx))
  {
    //        QCPGraph *g= m_graphs.value(idx);
    QCPGraph *g = m_graphs.take(idx);
    if (removeGraph(g))
    {
      // TODO: why actually del but return false?
      //            qDebug() << "removeGraph fail: " << idx;
    }
  }
  else
  {
    qDebug() << "m_graphs not exist:" << idx;
  }

  replot();
}

MyQCPGraph *TPPlot::getGraph(QString refrowidx, int dir, int width)
{
  // refrowidx:
  // width: line width, default 1
  QPen graphPen;
  QCPGraph *g;
  MyQCPGraph *myGraph;
  if (!m_graphs.contains(refrowidx))
  { // new graphs when not exist
    g = addGraph(xAxis, yAxis);
    myGraph = static_cast<MyQCPGraph *>(g);
    // myGraph = new MyQCPGraph(xAxis, yAxis);
    connect(myGraph, &MyQCPGraph::datasSetted, this, &TPPlot::onDatasSetted);
    int R = rand() % 245 + 10;
    int G = rand() % 245 + 10;
    int B = rand() % 245 + 10;
    graphPen = newColorPen(R, G, B, width);
    myGraph->setPen(graphPen);
    myGraph->setLineStyle(QCPGraph::lsLine);
    // registerPlottable();
    if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive))
    {
      // total graph
      myGraph->setLayer(LAYER_TOTAL);
      myGraph->setVisible((m_tpgrouptype == TPGroup::GroupMode::Total));
      mTotalGraph = myGraph;
    }
    else if ((refrowidx.contains(GRAPH_TX, Qt::CaseSensitive)) ||
             (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive)))
    {

      myGraph->setLayer(LAYER_DIR);
      myGraph->setVisible((m_tpgrouptype == TPGroup::GroupMode::Direction));
      if (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive))
      {
        mDirTxGraph = myGraph;
        mDirTxGraph->setDirection(dir);
      }
      else
      {
        mDirRxGraph = myGraph;
        mDirRxGraph->setDirection(dir);
      }
      // TODO: comment
    }
    else
    {
      // normal TP graph
      myGraph->setLayer(LAYER_MAIN);
      myGraph->setVisible((m_tpgrouptype == TPGroup::GroupMode::Detail));
      myGraph->setDirection(dir);
    }
    m_graphs.insert(refrowidx, myGraph);
    QCPPlottableLegendItem *litm = legend->itemWithPlottable(myGraph);
    if (litm && !m_legends.contains(refrowidx))
    {
      m_legends.insert(refrowidx, litm);
    }
    if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive))
    {
      mTotalLegendItem = litm;
    }
    else if (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive))
    {
      mDirTxLegendItem = litm;
    }
    else if (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive))
    {
      mDirRxLegendItem = litm;
    }
    setTPGroupType(m_tpgrouptype);
  }
  else
  {
    // qDebug() << "//we already have it:" << refrowidx;
    myGraph = m_graphs.value(refrowidx);
  }
  // qDebug() << "myGraph: " << myGraph << " ,refrowidx: " << refrowidx << " ,dir: " << dir;
  myGraph->setName(refrowidx);
  calculateLegendItems();
  return myGraph;
}

MyQCPBars *TPPlot::getLostRateGraph(QString refrowidx, int dir)
{
  QPen graphPen;
  MyQCPBars *g_lostrate;
  if (!m_lostgraphs.contains(refrowidx))
  {
    QCPGraph *g;
    // get main graph's color
    if (m_graphs.contains(refrowidx))
    {
      // use same color as throughput chart
      g = m_graphs.value(refrowidx);
      graphPen = g->pen();
    }
    else
    {
      int R = rand() % 245 + 10;
      int G = rand() % 245 + 10;
      int B = rand() % 245 + 10;
      graphPen = newColorPen(R, G, B, 1);
    }

    g_lostrate = new MyQCPBars(xAxis, yAxis2);
    g_lostrate->setWidthType(QCPBars::wtAbsolute);
    g_lostrate->setWidth(20);
    connect(g_lostrate, &MyQCPBars::datasSetted, this,
            &TPPlot::onLostRateDatasSetted);
    if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive))
    {
      // total lost rate graph
      g_lostrate->setLayer(LAYER_TOTALOSTRATE);
      g_lostrate->setVisible(m_tpgrouptype == TPGroup::GroupMode::Total);
      mTotalLostGraph = g_lostrate;
    }
    else if ((refrowidx.contains(GRAPH_TX, Qt::CaseSensitive)) ||
             (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive)))
    {
      g_lostrate->setLayer(LAYER_DIRLOSTRATE);
      g_lostrate->setVisible(m_tpgrouptype == TPGroup::GroupMode::Direction);
      if (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive))
      {
          mDirTxLostGraph = g_lostrate;
          mDirTxLostGraph->setDirection(dir);
      }
      else
      {
          mDirRxLostGraph = g_lostrate;
          mDirRxLostGraph->setDirection(dir);
      }
    }
    else
    {
      // normal lost rate graph
      g_lostrate->setLayer(LAYER_LOSTRATE);
      g_lostrate->setVisible(m_tpgrouptype == TPGroup::GroupMode::Detail);
    }
    QPen redPen = newColorPen(255, 0, 0, 2);
    g_lostrate->setPen(redPen);
    QColor c = graphPen.color();
    c.setAlpha(120); // 0~255, 255 not transparency
    g_lostrate->setBrush(c);

    m_lostgraphs.insert(refrowidx, g_lostrate);
    // qDebug() << "legend->itemCount:" << legend->itemCount();
    // QCPAbstractLegendItem *litm = legend->item(legend->itemCount() - 1);
    QCPAbstractLegendItem *litm = legend->itemWithPlottable(g_lostrate);
    if (litm && !m_lostratelegends.contains(refrowidx))
    {
      m_lostratelegends.insert(refrowidx, litm);
    }
    if (refrowidx.contains(GRAPH_TOTAL, Qt::CaseSensitive))
    { // Total legend
        mTotalLostLegendItem = litm;
    }
    else if (refrowidx.contains(GRAPH_TX, Qt::CaseSensitive))
    {
        mDirTxLostLegendItem = litm;
    }
    else if (refrowidx.contains(GRAPH_RX, Qt::CaseSensitive))
    {
        mDirRxLostLegendItem = litm;
    }
    setTPGroupType(m_tpgrouptype);
  }
  else
  {
    g_lostrate = m_lostgraphs.value(refrowidx);
  }

  g_lostrate->setName(refrowidx + " Lost Rate");
  calculateLegendItems();
  return g_lostrate;
}

void TPPlot::clear()
{
  QMutexLocker locker(&m_mutex);
  if (m_replottimer)
    m_replottimer->stop(); // 先叫計時器閉嘴
  setUpdatesEnabled(false);

  // clear tp data
  QMap<QString, MyQCPGraph *>::iterator it = m_graphs.begin();
  while (it != m_graphs.end())
  {
    MyQCPGraph *graph = it.value();
    if ((it.key().contains(GRAPH_TOTAL)) ||
        (it.key().contains(GRAPH_TX)) ||
        (it.key().contains(GRAPH_RX)))
    {
      // qDebug() << it.key() << " not delete, only clear data";
      graph->data()->clear(); // clear data
      ++it;                   // 沒刪除則繼續下一個
    }
    else
    {
      if (graph)
      {
        // qDebug() << it.key() << " removed";
        removeGraph(graph);
        // delete graph; // 如果是自訂非 QObject 物件才需要
      }
      // Also remove the corresponding legend item to avoid dangling pointer
      m_legends.remove(it.key());
      // 使用 erase 刪除當前節點，並回傳下一個有效的迭代器
      it = m_graphs.erase(it);
    }
  }

  QMap<QString, MyQCPBars *>::iterator itl = m_lostgraphs.begin();
  while (itl != m_lostgraphs.end())
  {
    MyQCPBars *graph = itl.value();
    if ((itl.key().contains(GRAPH_TOTAL)) ||
        (itl.key().contains(GRAPH_TX)) ||
        (itl.key().contains(GRAPH_RX)))
    {
      // qDebug() << "lostgraphs:" << itl.key() << " not delete, only clear data";
      graph->data()->clear(); // clear data
      ++itl;
    }
    else
    {
      if (graph)
      {
        // qDebug() << "lostgraphs:" << itl.key() << " removed";
        removePlottable(graph);
        // delete graph; // 如果是自訂非 QObject 物件才需要
      }
      // Also remove the corresponding legend item to avoid dangling pointer
      m_lostratelegends.remove(itl.key());
      // 使用 erase 刪除當前節點，並回傳下一個有效的迭代器
      itl = m_lostgraphs.erase(itl);
    }
  }

  // axis reset
  xAxis->setRange(0, m_xAxisMaxDefault);
  yAxis->setRange(0, m_yAxisMaxDefault);

  setStartTime(QDateTime());

  setUpdatesEnabled(true);
  replot(QCustomPlot::rpImmediateRefresh); // when no graph, replot will cause
                                           // plot area shrink
  if (m_replottimer)
    m_replottimer->start(100); // 清理完畢再開啟
}

void TPPlot::setXRangeUpper(double upper)
{
  // qDebug() << "setXRangeUpper:" << QString::number(upper);
  xAxis->setRangeUpper(upper);
  replot();
}

void TPPlot::initCustomPlot()
{
  this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes |
                        QCP::iSelectLegend | QCP::iSelectPlottables);
  this->axisRect()->setupFullAxesBox();
  this->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
  // this->axisRect()->setRangeDrag(Qt::Horizontal);
  this->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
  // this->axisRect()->setRangeZoom(Qt::Horizontal); //TODO: yAxis can not zoom,
  axisRect()->setRangeZoomAxes(xAxis, yAxis);
  axisRect()->setRangeDragAxes(xAxis, yAxis);

  // this->setAutoAddPlottableToLegend(true); // when adding a plottable,
  // automatically adds the QCPAbstractLegendItem to the legend
  // x Axis
  QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
  timeTicker->setTimeFormat("%h:%m:%s");
  xAxis->setTicker(timeTicker);
  // set axis Label
  xAxis->setLabel("Time(Sec)");
  // yAxis->setRangeLower(0);        // 下限不能低于 0, Drag still show <0
  yAxis2->setLabel("Lost Rate(%)");
  yAxis2->setTickLabels(true);

  // set axis range
  //  TODO: update range by throughput/time
  xAxis->setRange(0, m_xAxisMaxDefault);
  yAxis->setRange(0, m_yAxisMaxDefault);
  yAxis2->setRange(0, 100);
  yAxis2->setTickLabelColor(Qt::blue);
  yAxis2->setLabelColor(Qt::blue);
  // legend
  legend->setVisible(true);
  // legend->setBorderPen(Qt::NoPen); // no Border line
  // legend->setBrush(Qt::NoBrush); // no background
  legend->setBorderPen(QPen(Qt::black, 1)); // Black border with 1px width
  legend->setBrush(QBrush(QColor(255, 255, 255, 200)));
  // connect(legend, &QCPLegend::layerChanged)
  QFont legendFont = font();
  legendFont.setPointSize(8);
  legend->setFont(legendFont);
  legend->setSelectedFont(legendFont);
  // legend box shall not be selectable, only legend items
  legend->setSelectableParts(QCPLegend::spItems);

  if (1)
  {
    // TODO: not good on layout on Detail mode!! small, not extend the row
    axisRect()->setMinimumSize(100, 200);
    // Add the QCustomPlot legend to the container
    QCPLayoutGrid *subLayout = new QCPLayoutGrid();
    subLayout->setMinimumSize(100, 200);
    // TODO: position the legend outside of the graph!!
    plotLayout()->addElement(0, 1, subLayout);
    plotLayout()->setColumnStretchFactor(0, 1);   // col 0
    plotLayout()->setColumnStretchFactor(1, 0.1); // col 1
    plotLayout()->setRowStretchFactor(0, 1);      // row 0
    qDebug() << "plotLayout height: " << plotLayout()->rect().height();
    subLayout->addElement(0, 0, legend); // row 0, col 0
    // add spacer，let legend align up
    spacer = new QCPLayoutElement(this);
    spacer->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    subLayout->addElement(1, 0, spacer);
    // spacer take all rest space，let legend in smallest hight
    subLayout->setRowStretchFactor(0, 0.001); // legend row as small => all legend item Alignment to top
    subLayout->setRowStretchFactor(1, 1.0);   // spacer row take all space

    calculateLegendItems();
  }
  // make left and bottom axes transfer their ranges to right and top axes:
  //    connect(xAxis, SIGNAL(rangeChanged(QCPRange)), xAxis2,
  //    SLOT(setRange(QCPRange))); connect(yAxis,
  //    SIGNAL(rangeChanged(QCPRange)), yAxis2, SLOT(setRange(QCPRange)));
  // following cause strange behavior
  // connect(xAxis, qOverload<const QCPRange&>(&QCPAxis::rangeChanged), this,
  // &TPPlot::onXAxisRangeChanged);
  connect(this, &QCustomPlot::selectionChangedByUser, this,
          &TPPlot::selectionChanged);
}

QPen TPPlot::newColorPen(int r, int g, int b, int width)
{
  QPen graphPen;
  graphPen.setColor(QColor(r, g, b));
  graphPen.setWidthF(width);
  return graphPen;
}

void TPPlot::updateXAxisRange(double mintime, double maxtime)
{
  if (xAxis->range().lower > mintime)
  {
    mintime = xAxis->range().lower;
  }
  else
  {
    mintime = mintime * 0.9;
  }
  // if ((xAxis->range().upper / maxtime)>1.1){
  //     maxtime = xAxis->range().upper;
  // }else{
  //     maxtime = maxtime *1.1;
  // }
  if (maxtime < m_maxX)
  {
    maxtime = m_maxX;
  }
  if (maxtime > (xAxis->range().upper + m_interval))
  {
    qDebug() << "update xAxis min:" << QString::number(mintime)
             << " ,max:" << QString::number(maxtime);
    xAxis->setRange(mintime, maxtime); // show all data on plot
  }
  // xAxis->setRange(maxtime, m_xAxisMaxDefault, Qt::AlignRight);// not good!!
  // xAxis->setRange(0, maxtime, Qt::AlignRight); // bed, not show the graph
  // xAxis->setRange(mintime, maxtime, Qt::AlignCenter);
}

void TPPlot::updateYAxisRange(double minvalue, double maxvalue)
{
  if (yAxis->range().lower < minvalue)
  {
    minvalue = yAxis->range().lower;
  }
  else
  {
    minvalue = minvalue - (0.1 * minvalue);
  }
  if ((yAxis->range().upper / maxvalue) > 1.1)
  {
    maxvalue = yAxis->range().upper;
  }
  else
  {
    if (maxvalue < 100)
    {
      // maxvalue = maxvalue + (0.5*maxvalue);
      maxvalue = maxvalue * 1.5;
    }
    else
    {
      // maxvalue = maxvalue + (0.1*maxvalue);
      maxvalue = maxvalue * 1.1;
    }
  }
  yAxis->setRange(minvalue, maxvalue);
}

QVector<QCPGraphData>
TPPlot::convertQMapToQVector(const QMap<double, double> &map)
{
  // Not good for many QCPGraph data in!!
  // TODO: when many data, is this good in preformance?
  QVector<QCPGraphData> vector;
  for (auto it = map.constBegin(); it != map.constEnd(); ++it)
  {
    QCPGraphData data;
    data.key = it.key();
    data.value = it.value();
    vector.append(data);
    // QCoreApplication::processEvents(QEventLoop::AllEvents);
  }
  return vector;
}

// QSharedPointer<QCPGraphDataContainer>
// TPPlot::sumGraphData(const QSharedPointer<QCPGraphDataContainer> &data1,
//                      const QSharedPointer<QCPGraphDataContainer> &data2)
// {
//   QSharedPointer<QCPGraphDataContainer> result(new QCPGraphDataContainer);
//   auto it1 = data1->constBegin();
//   auto it2 = data2->constBegin();

//   double sumvalue = 0.0;
//   while (it1 != data1->constEnd() && it2 != data2->constEnd())
//   {
//     if (it1->key < it2->key)
//     {
//       result->add(QCPGraphData(it1->key, it1->value));
//       ++it1;
//     }
//     else if (it1->key > it2->key)
//     {
//       result->add(QCPGraphData(it2->key, it2->value));
//       ++it2;
//     }
//     else
//     {
//       sumvalue = it1->value + it2->value;
//       result->add(QCPGraphData(it1->key, sumvalue));
//       ++it1;
//       ++it2;
//     }
//   }
//   while (it1 != data1->constEnd())
//   {
//     result->add(QCPGraphData(it1->key, it1->value));
//     ++it1;
//   }
//   while (it2 != data2->constEnd())
//   {
//     result->add(QCPGraphData(it2->key, it2->value));
//     ++it2;
//   }

//   return result;
// }

QSharedPointer<QCPBarsDataContainer>
TPPlot::sumLostGraphData(const QSharedPointer<QCPBarsDataContainer> &data1,
                         const QSharedPointer<QCPBarsDataContainer> &data2)
{
  QSharedPointer<QCPBarsDataContainer> result(new QCPBarsDataContainer);
  auto it1 = data1->constBegin();
  auto it2 = data2->constBegin();

  while (it1 != data1->constEnd() && it2 != data2->constEnd())
  {
    if (it1->key < it2->key)
    {
      result->add(QCPBarsData(it1->key, it1->value));
      ++it1;
    }
    else if (it1->key > it2->key)
    {
      result->add(QCPBarsData(it2->key, it2->value));
      ++it2;
    }
    else
    {
      result->add(QCPBarsData(it1->key, it1->value + it2->value));
      ++it1;
      ++it2;
    }
  }
  while (it1 != data1->constEnd())
  {
    result->add(QCPBarsData(it1->key, it1->value));
    ++it1;
  }
  while (it2 != data2->constEnd())
  {
    result->add(QCPBarsData(it2->key, it2->value));
    ++it2;
  }

  return result;
}

void TPPlot::calculateLegendItems()
{
  // get the legend size and calculate the number of items can show
  QSize legendSize = legend->rect().size(); //->minimumOuterSizeHint();
  QSize legendSpacerSize = spacer->rect().size();
  // calculate Max Legend Items
  int itemHeight =
      legend->font().pointSize() + 9; // approximate height of each item
  int itemsFit = legendSize.height() / itemHeight;

  qDebug() << "Legend height:" << legendSize.height() << " itemHeight:" << itemHeight
           << " itemsFit:" << itemsFit
           << " legendSpacerSize height:" << legendSpacerSize.height()
           << " plotLayout height: " << plotLayout()->rect().height();
  // qDebug() << "Legend rect:" << legend->rect(); qDebug() <<
  // "Approximate number of items that can fit:" << itemsFit;
  if (m_tpgrouptype == TPGroup::GroupMode::Total)
  {
    emit sigLegendCount(1);
  }
  else if (m_tpgrouptype == TPGroup::GroupMode::Direction)
  {
    emit sigLegendCount(2);
  }
  else
  {
    // qDebug() << "sigLegendCount:" << itemsFit << ",itemHeight:" << itemHeight
    //          << ", legend height" << legendSize.height();
    emit sigLegendCount(itemsFit);
  }
}

QCPDataContainer<QCPGraphData>::const_iterator
TPPlot::findKeyValue(const QCPDataContainer<QCPGraphData> &container,
                     double key)
{
  // Use findBegin to get an iterator to the data point with the closest key
  auto it = container.findBegin(key, true);
  // if (it != container.constEnd() && it->key == key) {
  if (it != container.constEnd())
  {
    qDebug() << "it->key:" << it->key << " key:" << key;
    if (qFuzzyCompare(it->key, key))
    {
      return it; // Found the exact key
    }
  }
  return container.constEnd(); // Key not found
}
