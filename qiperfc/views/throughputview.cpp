#include "throughputview.h"
#include "ui_throughputview.h"

#include <QMessageBox>
#include <QModelIndexList>
#include <QScrollBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>

#include <QDebug>

#include "../src/tooltipeventfilter.h"
#include "comm.h"
#include "../src/nmessagebox.h"


// ThroughputView::ThroughputView(QIperfC *main, QWidget *parent) : AbstractView(parent)
ThroughputView::ThroughputView(QAction *aCopy, QAction *aPaste, QAction *aDelete,
                               QAction *aCopyText, QSettings *cfg,
                               int tpgroup, QString sunit,
                               QWidget *parent) :
    AbstractView(parent),
    ui(new Ui::ThroughputView), m_actionCopy(aCopy),m_actionPaste(aPaste),
    m_actionDelete(aDelete),m_actionCopyText(aCopyText), m_tpgrouptype(tpgroup),
    m_tpunit(sunit)
//, m_main(main)
{
    m_iperfwrapper = new IperfWrapper();
    connect(m_iperfwrapper, &IperfWrapper::debuginfo, this, &ThroughputView::onDebuginfo);
    ui->setupUi(this);
    initThroughputChart();
    m_clipboard = QApplication::clipboard();

    dlgiperf = new DlgIperf(m_tpmgr, cfg, this); //add/edit iperf config dialog
    initMenus();
    connect(this, &ThroughputView::showGrouptype, this, &ThroughputView::setTPGroupType);
}

ThroughputView::~ThroughputView()
{
    delete ui;
}

int ThroughputView::rootChildCount()
{
    return m_tpmgr->rootChildCount();
}

QByteArray ThroughputView::savedata()
{
    return m_tpmgr->savedata();
}

QStringList ThroughputView::getPCs()
{
    return m_tpmgr->getPCs();
}

void ThroughputView::reset()
{
    m_tpmgr->reset();
}

QList<TP *> ThroughputView::getChilds(bool showAll)
{
    return m_tpmgr->getChilds(showAll);
}

bool ThroughputView::addEndpoint(QString mgr, QString mdata)
{   //update dlgiperf's manager data
    if (dlgiperf){
        if (dlgiperf->add(mgr, mdata)){
            dlgiperf->updateUI();
            return true;
        }
    }
    return false;
}

void ThroughputView::doClear()
{
    if (m_tpmgr->rootChildCount()>0) {
        m_tpmgr->clear();
        // ui->tv_throughput->collapseAll(); //this cause when start running, group item will collapse
    }
    m_tpplot->clear();

}

QPixmap ThroughputView::toPixmap(int width, int height, double scale)
{
    return m_tpplot->toPixmap(width, height, scale);
}

void ThroughputView::addComment(QString midx, QString comment)
{
    m_tpmgr->addComment(midx, comment);
}

QDateTime ThroughputView::getStartTime()
{
    return m_starttime;
}

bool ThroughputView::getTP(QString &tpvalue, QString &lostrate)
{
    // get throughput
    if (m_tpgrouptype ==  static_cast<int>(TPGroup::GroupMode::Total)){
        TP *tp = m_tpmgr->getRootItem();
        tpvalue = tp->getThroughput();
        lostrate = tp->getLostRate();
    }else{
        QList<TP*> ds = m_tpmgr->getChilds();
        double dtp=0.0;
        double dlr=-1.0;
        foreach(auto d, ds){
            if (d->getDataType()==TPMgrData::config){
                dtp = dtp + d->getThroughput().toDouble();
                dlr = dlr + d->getLostRate().toDouble();
            }
        }
        tpvalue = QString::number(dtp);
        lostrate = QString::number(dlr);
    }
    return true;
}

void ThroughputView::onCopy()
{
    if (ui->tv_throughput->hasFocus()){
        QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
        //        qDebug() << "onCopy:" << idxs;
        TP *tp;
        QString s="";
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            s = s + "\n" + tp->getJsonData();
        }
        m_clipboard->setText(s);
    }else {
        qDebug() << "tv_throughput no hasFocus";
    }

}

void ThroughputView::onPaste()
{
    if (ui->tv_throughput->hasFocus()){
        QString clip = m_clipboard->text();
        m_tpmgr->onPaste(clip);
    }else {
        qDebug() << "tv_throughput no hasFocus";
    }
}

void ThroughputView::onDelete()
{
    QModelIndex curIdx = ui->tv_throughput->selectionModel()->currentIndex();
    if (curIdx.isValid()){
        TP *tp = m_tpmgr->getItem(curIdx);
        if (tp->getDataType() == TPMgrData::config){
            // remove releative iperf3 log file
            QString client = tp->getBindKey(false); //client
            QString server = tp->getBindKey(); //server
            QString d = getStartTime().toString(DATETIME_NOW_FORMAT);

            if (tp->haveChilds()){
                foreach(TP *p, tp->getChilds()){
                    m_tpplot->del(p->getID());
                }
            }
            m_tpmgr->del(curIdx);
            // TODO: should/how we modify remain item's idx??
            // signal remove files
            QStringList fs;
            fs.append(d + QDir::separator() + client + ".log");
            fs.append(d + QDir::separator() + server + ".log");
            emit deleteFiles(fs);
        }
    }
}

void ThroughputView::onCopyText()
{
    if (ui->tv_throughput->hasFocus()){
        QModelIndexList mls = ui->tv_throughput->selectionModel()->selectedRows();
        if (mls.length()>0){
            QModelIndex idx = mls[0]; // first model index
            QPoint globalPos = QCursor::pos();
            QPoint widgetPos = ui->tv_throughput->mapFromGlobal(globalPos);
            int col = ui->tv_throughput->columnAt(widgetPos.x());

            TP *tp;
            tp = m_tpmgr->getItem(idx);
            m_clipboard->setText(tp->data(col).toString());
        }
    }else {
        qDebug() << "onCopyText: tv_throughput not hasFocus";
    }
}

void ThroughputView::onAddIperf()
{
    // add iperf test pair
    dlgiperf->updateUI();
    dlgiperf->setExcIdx(QModelIndex());//new
    int rc = dlgiperf->exec();// show dlgiperf
    if (rc == QDialog::Accepted){
        QString rs= dlgiperf->getJsonCfg();
        AddIperf(rs);
        emit updateActionsSave(true);
    }
}

void ThroughputView::AddIperf(QString cfg)
{
    // cfg is json format in string
    // qDebug()<< "ThroughputView onAddIperf: \n" << cfg;
    m_tpmgr->add(cfg);
}

void ThroughputView::onPairEdit()
{
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
    if (idx.isValid()){
        onItemDClicked(idx);
    }
}

void ThroughputView::onPairDelete()
{
    //TODO: do not use this, use onDelete()
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
    if (idx.isValid()){
        m_tpmgr->del(idx);
    }
}

void ThroughputView::onPairSwap()
{
    QModelIndexList mls= ui->tv_throughput->selectionModel()->selectedRows();
    foreach (QModelIndex midx, mls) {
        m_tpmgr->swapDirection(midx);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void ThroughputView::onPairSwapIP()
{
    QModelIndexList mls= ui->tv_throughput->selectionModel()->selectedRows();
    foreach (QModelIndex midx, mls) {
        m_tpmgr->swapIPDirection(midx);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void ThroughputView::setStartTime(QDateTime startTime)
{
    m_starttime = startTime;
    m_tpplot->setStartTime(startTime);
}

void ThroughputView::onAddTPdata(QString midx, QString sInterval, QString idx, QString value, QString unit, QString dir, QString pkt_lost, QString pkt_total)
{
    m_tpmgr->addTPdata(midx, sInterval, idx, value, unit, dir, pkt_lost, pkt_total);
}

void ThroughputView::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas, QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates)
{
    m_tpplot->onUpdateTPDatas(refrow, timedatas, valuedatas, packetlosts, packettotals, lostrates);
}

void ThroughputView::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    //update iperf throughput from websocket client
    m_tpmgr->onIperfTPdata(refrow, sInterval, datas);
}

void ThroughputView::onUpdateTPCfg(QByteArray tpcfg)
{
    m_tpmgr->loaddata(tpcfg);
}

void ThroughputView::onUpdateTPUnit(QString suint)
{
    //this update by options/iperf/throughput unit
    m_tpmgr->setTPUint(suint);
    m_tpplot->setTPUint(suint);
}

void ThroughputView::onGroupCopyItem(bool checked)
{
    Q_UNUSED(checked)
    QList<QString> a;
    QModelIndexList idxs= ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        foreach (QModelIndex midx, idxs) {
            QString idx = midx.data().toString(); // Rx, Tx ...
            qDebug() << "idx: " << idx << " midx:" << midx;
            TP *itm = m_tpmgr->getItemByIdx(idx);
            if (itm){
                a.append(idx + ":" + itm->data(0).toString());
            }
        }
    }
    m_clipboard->setText(a.join(", "));
}

void ThroughputView::setShowGroupTotal(bool bShow)
{
    Q_UNUSED(bShow)
    //set show group Total
    // m_tpgrouptype =  static_cast<int>(TPGroup::GroupMode::Total);
    // m_tpplot->setShowGroup(bShow);
    emit showGrouptype(static_cast<int>(TPGroup::GroupMode::Total));
}

void ThroughputView::setShowGroupPair(bool bShow)
{
    Q_UNUSED(bShow)
    // m_tpgrouptype =  static_cast<int>(TPGroup::GroupMode::Detail);
    emit showGrouptype(static_cast<int>(TPGroup::GroupMode::Detail));
}

void ThroughputView::setShowGroupDir(bool bShow)
{
    Q_UNUSED(bShow)
    // m_tpgrouptype =  static_cast<int>(TPGroup::GroupMode::Direction);
    emit showGrouptype(static_cast<int>(TPGroup::GroupMode::Direction));
}

void ThroughputView::setShowGroupComment(bool bShow)
{
    Q_UNUSED(bShow)
    // m_tpgrouptype =  static_cast<int>(TPGroup::GroupMode::Comment);
    emit showGrouptype(static_cast<int>(TPGroup::GroupMode::Comment));
}

void ThroughputView::setTPGroupType(int grouptype)
{
    m_tpgrouptype = grouptype;
    if (grouptype == static_cast<int>(TPGroup::GroupMode::Direction)){
        dlgiperf->setBiDirStatus(false);
    }else{
        dlgiperf->setBiDirStatus(true);
    }
    m_tpmgr->setTPGroupType(grouptype);
    //TODO: m_tpplot->setTPGroupType(grouptype);
}

void ThroughputView::getRawData(bool checked)
{
    Q_UNUSED(checked)
    //selected graph
    QString jstr="";
    foreach (QCPGraph *g , m_tpplot->selectedGraphs()){
        if (!jstr.isEmpty()){
            jstr.append(",");
        }
        jstr.append(getGraphDataToJsonStr(g));
    }
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(jstr);
}

void ThroughputView::onSaveImg(bool checked)
{
    Q_UNUSED(checked)
    //
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save throughput plotchart to png"),
                                                    path, tr(QIPERF_EXT_FILTER_PNG));
    if (!fileName.isEmpty()) {
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if (ext.compare(QIPERF_EXT_PNG)!=0){
            fileName = fi.path()+ QDir::separator() + fi.baseName() + "."+ QIPERF_EXT_PNG;
        }
        m_tpplot->savePng(fileName);
    }
}

void ThroughputView::setXRangeUpper(double upper)
{
    if (m_tpplot){
        m_tpplot->setXRangeUpper(upper);
    }
}

void ThroughputView::setInterval(int interval)
{
    emit updateInterval(interval);
}

void ThroughputView::onTestStarted()
{
    emit startstop(true);
}

void ThroughputView::onTestStoped(int err)
{
    Q_UNUSED(err)
    emit startstop(false);
}

void ThroughputView::initMenus()
{
    m_aEnable = new QAction("Enable select item");
    connect(m_aEnable, &QAction::triggered, this, &ThroughputView::onEnableItem);
    m_aDisable = new QAction("Disable select item");
    connect(m_aDisable, &QAction::triggered, this, &ThroughputView::onDisableItem);
    m_aDisable->setEnabled(false);

    m_actionClientArgs = new QAction("Copy Iperf client cmd");
    connect(m_actionClientArgs, &QAction::triggered, this, &ThroughputView::copyClientArgs);
    m_actionServerArgs = new QAction("Copy Iperf server cmd");
    connect(m_actionServerArgs, &QAction::triggered, this, &ThroughputView::copyServerArgs);

    m_tpmenu = new QMenu(); // config throughput pair right click menu
    m_tpmenu->addAction(m_actionCopy);
    m_tpmenu->addAction(m_actionPaste);
    m_tpmenu->addAction(m_actionDelete);
    m_tpmenu->addSeparator();
    m_tpmenu->addAction(m_actionCopyText);
    m_tpmenu->addSeparator();
    m_tpmenu->addAction(m_aEnable);
    m_tpmenu->addAction(m_aDisable);
    m_tpmenu->addSeparator();
    m_tpmenu->addAction(m_actionClientArgs);
    m_tpmenu->addAction(m_actionServerArgs);

    m_tpgroupmenu = new QMenu(); //config group : total, direction, comm item
    m_actionGroupCopyItem = new QAction("Copy Item");
    connect(m_actionGroupCopyItem, &QAction::triggered, this, &ThroughputView::onGroupCopyItem);
    m_tpgroupmenu->addAction(m_actionGroupCopyItem);
    //right menu
    m_rightmenu = new QMenu(this);
    m_menuGroup = new QMenu("Group", this);
    m_rightmenu->addMenu(m_menuGroup);

    m_actionGroupTotal = new QAction("Total");
    m_actionGroupTotal->setCheckable(true);
    connect(m_actionGroupTotal, &QAction::triggered, this, &ThroughputView::setShowGroupTotal);
    m_actionGroupPair = new QAction("Iperf Pair");
    m_actionGroupPair->setCheckable(true);
    connect(m_actionGroupPair, &QAction::triggered, this, &ThroughputView::setShowGroupPair);
    m_actionGroupDir = new QAction("Direction");
    m_actionGroupDir->setCheckable(true);
    connect(m_actionGroupDir, &QAction::triggered, this, &ThroughputView::setShowGroupDir);
    m_actionGroupComment = new QAction("Comment(TODO)");
    m_actionGroupComment->setCheckable(true);
    connect(m_actionGroupComment, &QAction::triggered, this, &ThroughputView::setShowGroupComment);

    m_actionRawData = new QAction("Copy plotchart Raw Data");
    connect(m_actionRawData, &QAction::triggered, this, &ThroughputView::getRawData);
    m_actionSaveImg = new QAction("Save to Image");
    connect(m_actionSaveImg, &QAction::triggered, this, &ThroughputView::onSaveImg);
    m_actionAbout = new QAction("About");
    connect(m_actionAbout, &QAction::triggered, this, &ThroughputView::aboutQCustomPlot);

    m_menuGroup->addAction(m_actionGroupTotal);
    m_menuGroup->addAction(m_actionGroupPair);
    m_menuGroup->addAction(m_actionGroupDir);
    m_menuGroup->addAction(m_actionGroupComment);
    m_rightmenu->addSeparator();
    m_rightmenu->addAction(m_actionSaveImg);
    m_rightmenu->addAction(m_actionAbout);
}

void ThroughputView::onPlotContextMenuRequest(QPoint pos)
{
    if (m_tpgrouptype ==  static_cast<int>(TPGroup::GroupMode::Total)){
        m_actionGroupTotal->setChecked(true);
    }else {
        m_actionGroupTotal->setChecked(false);
    }
    if (m_tpgrouptype ==  static_cast<int>(TPGroup::GroupMode::Detail)){
        m_actionGroupPair->setChecked(true);
    }else {
        m_actionGroupPair->setChecked(false);
    }
    if (m_tpgrouptype ==  static_cast<int>(TPGroup::GroupMode::Direction)){
        m_actionGroupDir->setChecked(true);
    }else {
        m_actionGroupDir->setChecked(false);
    }
    if (m_tpgrouptype ==  static_cast<int>(TPGroup::GroupMode::Comment)){
        m_actionGroupComment->setChecked(true);
    }else {
        m_actionGroupComment->setChecked(false);
    }

    if (m_tpplot->selectedGraphs().count()>0){
        m_rightmenu->insertAction(m_actionAbout, m_actionRawData);
    }
    m_rightmenu->insertSeparator(m_actionAbout);
    m_rightmenu->popup(m_tpplot->mapToGlobal(pos));
}

void ThroughputView::onSelectedTPitem(QString idx)
{
    //set TP item selected
    QModelIndex indexToSelect =  m_tpmgr->setSelectItem(idx);
    // Get the selection model
    QItemSelectionModel *selectionModel = ui->tv_throughput->selectionModel();
    selectionModel->select(indexToSelect, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    // Optionally, make the selection visible
    ui->tv_throughput->scrollTo(indexToSelect);
}

void ThroughputView::onTPUTContextMenu(QPoint pos)
{
    // if select multi items
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>1){
        m_aEnable->setEnabled(true);
        m_aDisable->setEnabled(true);
    } else {
        //TODO: check select item status enable/disable menu item
        QModelIndex midx = ui->tv_throughput->indexAt(pos);
        TP *tp = m_tpmgr->getItem(midx);
        if (tp){
            if (tp->getEnabled()){
                m_aEnable->setEnabled(false);
                m_aDisable->setEnabled(true);
            }else{
                m_aEnable->setEnabled(true);
                m_aDisable->setEnabled(false);
            }
            // if ((tp->getDataType()!=TPMgrData::config) &&
                // (tp->getDataType()!=TPMgrData::TP)){
            if (tp->getDataType()!=TPMgrData::config){
                //don't show menu on not supported item
                m_tpgroupmenu->popup(ui->tv_throughput->mapToGlobal(pos));
                return;
            }
            if (tp->getDataType()!=TPMgrData::config){
                m_actionClientArgs->setEnabled(false);
                m_actionServerArgs->setEnabled(false);
            }else{
                m_actionClientArgs->setEnabled(true);
                m_actionServerArgs->setEnabled(true);
            }
        }else{
            //don't show menu on no item
            return;
        }
    }
    //show right menu
    m_tpmenu->popup(ui->tv_throughput->mapToGlobal(pos));
}

void ThroughputView::onTPDataUpdate(const QModelIndex &parent, int first, int last)
{
    qDebug() << "parent:" << parent << " first:" << QString::number(first) << " last:" << QString::number(last);
    //TODO: when throughput is running, add new TP item?
    // when new throughput data insert, this will trugger,
    bool bStart;
    if (m_tpmgr->rowCount()>0) {
        // TP *itm = m_tpmgr->getItem(parent);
        // if (itm){
        //     if (itm->getDataType() ==TPMgrData::TP){
        //         return;
        //     }
        // }
        bStart=false;
        emit updateActions(!bStart, bStart, !bStart);
    } else {
        qDebug() <<"onTPDataUpdate: m_tpmgr->rowCount <=0";
        emit updateActions(false, false, false);
    }
}

void ThroughputView::aboutQCustomPlot()
{
    QMessageBox::about(this, "About QCustomPlot", "QCustomPlot\n"
                                                  "Ver: "+ QString(QCUSTOMPLOT_VERSION_STR) + "\n"
                                                                                           "URL: https://www.qcustomplot.com/index.php/introduction");

}

void ThroughputView::onEnableItem(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedIndexes();
    if (idxs.length()>0){
        TP *tp;
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            if (!tp->isTPDataType()){
                continue;
            }
            tp->setEnabled(true);
        }
    }
}

void ThroughputView::onDisableItem(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        TP *tp;
        foreach(auto idx, idxs){
            tp = m_tpmgr->getItem(idx);
            if (!tp->isTPDataType()){
                continue;
            }
            tp->setEnabled(false);
        }
    }
}

void ThroughputView::copyClientArgs(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        TP *tp= m_tpmgr->getItem(idxs[0]);
        QString args = "" ;
        if (tp->getVersion()==3){
            args = m_iperfwrapper->toIperf3args(tp->getClientArgsMap());
        }else{
            args = m_iperfwrapper->toIperf2args(tp->getClientArgsMap());
        }
        m_clipboard->setText(args);
    }
}

void ThroughputView::copyServerArgs(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        TP *tp= m_tpmgr->getItem(idxs[0]);
        qDebug() << "copyServerArgs getVersion:" << tp->getVersion();
        QString args ="";
        if (tp->getVersion()==3){
            args = m_iperfwrapper->toIperf3args(tp->getServerArgsMap());
        } else {
            args = m_iperfwrapper->toIperf2args(tp->getServerArgsMap());
        }
        m_clipboard->setText(args);

    }
}

void ThroughputView::onItemDClicked(QModelIndex idx)
{
    TP *tp = m_tpmgr->getItem(idx);
    if (tp){
        if (tp->getDataType() == TPMgrData::config) {
            QPoint globalPos = QCursor::pos();
            QPoint widgetPos = ui->tv_throughput->mapFromGlobal(globalPos);
            int col = ui->tv_throughput->columnAt(widgetPos.x());
            if (col != TP::cols::comment) {
                // only iperf pair config can be edit
                dlgiperf->loadJsonCfg(tp->saveData());
                dlgiperf->setExcIdx(idx);
                int rc = dlgiperf->exec();// show dlgiperf
                if (rc == QDialog::Accepted){
                    QString rs= dlgiperf->getJsonCfg();
                    tp->loadData(rs);
                    m_tpmgr->setItem(idx, tp);
                }
            }else{
                qDebug() << "TODO: handle double click on column comment";
                // QMessageBox::information(this, "comment", tp->data(TP::cols::comment).toString());
                QVariant d = tp->data(TP::cols::comment);
                if (d.isValid() && ! d.isNull()) {
                    QString comment = d.toString();
                    if (!comment.isEmpty()){
                        NMessageBox *msg = new NMessageBox(QMessageBox::Information,
                                                           "comment", comment);
                        msg->show();
                    }
                }
            }
        }
    }
}

void ThroughputView::onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    //TODO: when delete item, this will cause actionDelete be disable => can not delete item continus
    bool bAct=false;
    if (selected.length()>0){
        bAct = true;
    }
    emit updateActionsEdit(bAct , bAct , bAct, bAct);
}

void ThroughputView::onVLegendScrollBarRange(int count)
{
    // m_vLegendScrollBar->setRange(0, count);
    m_vLegendScrollBar->setRange(0, count-10);
}

QString ThroughputView::getGraphDataToJsonStr(QCPGraph *graph)
{
    if (!graph) {
        return "";
    }

    QJsonArray dataArray;
    auto data = graph->data();

    for (auto it = data->constBegin(); it != data->constEnd(); ++it) {
        QJsonObject point;
        point["key"] = it->key;
        point["value"] = it->value;
        dataArray.append(point);
    }

    QJsonDocument doc(dataArray);
    return doc.toJson(QJsonDocument::Compact);
}

void ThroughputView::onDebuginfo(QString msg)
{
    qDebug() << "[ThroughputView]" << msg;
}

// void ThroughputView::onShowGrouptype(int grouptype)
// {
//     m_tpmgr->setTPGroupType(grouptype);
//     //TODO: plot
// }

void ThroughputView::initThroughputChart()
{
    // throughput chart
    m_vLegendScrollBar = new QScrollBar(Qt::Vertical, this);
    m_tpplot = new TPPlot(m_tpgrouptype, m_tpunit, ui->widget_console);
    // m_tpplot->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << "enable openGl:" << m_tpplot->openGl();
    m_tpplot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tpplot, &TPPlot::customContextMenuRequested, this, &ThroughputView::onPlotContextMenuRequest);
    connect(m_tpplot, &TPPlot::selectedTPitem, this, &ThroughputView::onSelectedTPitem);
    connect(m_tpplot, &TPPlot::sigLegendCount, this ,&ThroughputView::onVLegendScrollBarRange);
    connect(this, &ThroughputView::updateInterval, m_tpplot, &TPPlot::setInterval);
    connect(this, &ThroughputView::startstop, m_tpplot, &TPPlot::setTestStarted);
    ui->hl_console->addWidget(m_tpplot);
    ui->hl_console->addWidget(m_vLegendScrollBar);
    connect(m_vLegendScrollBar, &QScrollBar::valueChanged, m_tpplot, &TPPlot::onVLegendScrollChanged);
    // m_vLegendScrollBar->setRange(0, m_tpplot->legend->itemCount() - 10);
    onVLegendScrollBarRange(m_tpplot->legend->itemCount());

    m_tpmgr = new TPMgr(m_tpgrouptype, ui->tv_throughput, m_tpunit);
    // connect(m_tpmgr, &TPMgr::rowsInserted, this, &ThroughputView::onTPDataUpdate);
    // connect(m_tpmgr, &TPMgr::rowsRemoved, this, &ThroughputView::onTPDataUpdate);
    // connect(m_tpmgr, &TPMgr::IperfTPdata, m_tpplot, &TPPlot::onIperfTPdata);
    connect(m_tpmgr, &TPMgr::IperfTPdatas, m_tpplot, &TPPlot::onIperfTPdatas);
    connect(m_tpmgr, &TPMgr::debugMsg, this, &ThroughputView::onDebuginfo);

    ui->tv_throughput->setModel(m_tpmgr);
    // ui->tv_throughput->setHeaderHidden(true);// not show header column

    /* TODO: set specify column font size,
    // current not inherent other setting
    header = new CustomHeaderView(Qt::Horizontal, ui->tv_throughput);
    int s = header->getFontSize();
    header->setColumnSize(int(TP::mintp), s/2);
    header->setColumnSize(int(TP::maxtp), s/2);
    ui->tv_throughput->setHeader(header);

    // set specify column font size
//    QHeaderView *header = ui->tv_throughput->header();
//    QFont font = header->font();
//    qDebug() << "font size: " << font.pointSize();
//    font.setPointSize(28); // Set the desired font size
//    header->setStyleSheet(QString("QHeaderView::section:nth-child(%1) { font-size: %2pt; }").arg(1).arg(font.pointSize()));
    // end set font size

    */
    ui->tv_throughput->setColumnWidth(TP::cols::id, 100);
    ui->tv_throughput->setColumnWidth(TP::cols::server, 150);
    ui->tv_throughput->setColumnWidth(TP::cols::dir, 80);
    ui->tv_throughput->setColumnWidth(TP::cols::client, 150);
    ui->tv_throughput->setColumnWidth(TP::cols::lostrate, 110);
/*
    TooltipEventFilter* filter = new TooltipEventFilter(ui->tv_throughput);
    connect(filter, &TooltipEventFilter::doCopy, this, &ThroughputView::onCopy);
    connect(filter, &TooltipEventFilter::doPaste, this, &ThroughputView::onPaste);
    connect(filter, &TooltipEventFilter::doDelete, this, &ThroughputView::onDelete);
    ui->tv_throughput->viewport()->installEventFilter(filter);
*/
    ui->tv_throughput->setRootIsDecorated(true); //show folding icon

    ui->tv_throughput->setContextMenuPolicy(Qt::CustomContextMenu);  // custom right click menu
    connect(ui->tv_throughput, &QTreeView::customContextMenuRequested, this, &ThroughputView::onTPUTContextMenu);
    connect(ui->tv_throughput, &QTreeView::doubleClicked, this, &ThroughputView::onItemDClicked); //edit iperf config item on double click

    //TODO: slow update text
    tpdirdelegate = new TPDirDelegate(ui->tv_throughput);
    // tpdirdelegate = new TPDirDelegate(this); // this will not show dir picture
    ui->tv_throughput->setItemDelegateForColumn(TP::cols::dir, tpdirdelegate);

    nowrapdelegate = new NoWrapDelegate(ui->tv_throughput);
    ui->tv_throughput->setItemDelegateForColumn(TP::cols::comment, nowrapdelegate);

    //tpfoldingdelegate = new TPFoldingDelegate(ui->tv_throughput);
    //ui->tv_throughput->setItemDelegateForColumn(TP::cols::id, tpfoldingdelegate);
    ui->tv_throughput->setSelectionMode(QAbstractItemView::MultiSelection); //multiple selection
    QItemSelectionModel *ism = ui->tv_throughput->selectionModel();
    connect(ism, &QItemSelectionModel::selectionChanged, this, &ThroughputView::onTPselectionChanged);

}
