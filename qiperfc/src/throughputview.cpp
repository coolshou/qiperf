#include "throughputview.h"
#include "ui_throughputview.h"

#include <QMessageBox>
#include <QModelIndexList>
#include <QScrollBar>

#include "tooltipeventfilter.h"
#include "comm.h"
#include "nmessagebox.h"

// ThroughputView::ThroughputView(QIperfC *main, QWidget *parent) : AbstractView(parent)
ThroughputView::ThroughputView(QAction *aCopy, QAction *aPaste, QAction *aDelete,
                               QAction *aCopyText,  bool showgroup, QString sunit,
                               QWidget *parent) : AbstractView(parent)
    , ui(new Ui::ThroughputView), m_actionCopy(aCopy),m_actionPaste(aPaste),
    m_actionDelete(aDelete),m_actionCopyText(aCopyText), m_showgroup(showgroup),
    m_tpunit(sunit)
//, m_main(main)
{
    m_iperfwrapper = new IperfWrapper();
    ui->setupUi(this);
    initThroughputChart();
    m_clipboard = QApplication::clipboard();

    dlgiperf = new DlgIperf(m_tpmgr, this); //add/edit iperf config dialog
    initMenus();
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
    TP *tp = m_tpmgr->getItem(curIdx);
    if (tp->getDataType() == TPMgrData::DataType::config){
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
        fs.append(d+QDir::separator()+client+".log");
        fs.append(d+QDir::separator()+server+".log");
        emit deleteFiles(fs);
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
    // on_pair_add
    dlgiperf->updateUI();
    dlgiperf->setExcIdx(QModelIndex());//new
    int rc = dlgiperf->exec();// show dlgiperf
    if (rc == QDialog::Accepted){
        QString rs= dlgiperf->getJsonCfg();
        //        qDebug()<< "on_pairAdd: \n" << rs;
        m_tpmgr->add(rs);
        emit updateActionsSave(true);
    }
}

void ThroughputView::onPairEdit()
{
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
    onItemDClicked(idx);
}

void ThroughputView::onPairDelete()
{
    //TODO: do not use this, use onDelete()
    QModelIndex idx = ui->tv_throughput->selectionModel()->currentIndex();
    qDebug() << "TODO: onPairDelete:" << idx;
    m_tpmgr->del(idx);
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
    m_tpmgr->setTPUint(suint);
    m_tpplot->setTPUint(suint);
}

void ThroughputView::setShowGroup(bool bShow)
{
    //set show group
    m_showgroup = bShow;
    m_tpmgr->setShowGroup(m_showgroup);
    // TODO: m_tpplot
    m_tpplot->setShowGroup(m_showgroup);
    emit showGroup(m_showgroup);
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

    m_actionGroup = new QAction("Group");
    m_actionGroup->setCheckable(true);
    connect(m_actionGroup, &QAction::triggered, this, &ThroughputView::setShowGroup);
}

void ThroughputView::onPlotContextMenuRequest(QPoint pos)
{
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    m_actionGroup->setChecked(m_showgroup);
    menu->addAction(m_actionGroup);

    menu->addSeparator();
    //    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    //    {
    //        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignLeft));
    //        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
    //        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop | Qt::AlignRight));
    //        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignRight));
    //        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
    //    }
    //    else  // general context menu on graphs requested
    {
        //        if (ui->customPlot->selectedGraphs().size() > 0)
        //            menu->addAction("Remove selected graph", this, SLOT(removeSelectedGraph()));
        //        if (ui->customPlot->graphCount() > 0)
        //            menu->addAction("Remove all graphs", this, SLOT(removeAllGraphs()));
        menu->addAction("About", this, &ThroughputView::aboutQCustomPlot);
    }

    menu->popup(m_tpplot->mapToGlobal(pos));
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
    // Q_UNUSED(parent)
    // Q_UNUSED(first)
    // Q_UNUSED(last)
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
        QString args = m_iperfwrapper->toIperf3args(tp->getClientArgsMap());
        m_clipboard->setText(args);
    }
}

void ThroughputView::copyServerArgs(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->tv_throughput->selectionModel()->selectedRows();
    if (idxs.length()>0){
        TP *tp= m_tpmgr->getItem(idxs[0]);
        m_clipboard->setText(tp->getServerArgs());
        QString args = m_iperfwrapper->toIperf3args(tp->getServerArgsMap());
        m_clipboard->setText(args);

    }
}

void ThroughputView::onItemDClicked(QModelIndex idx)
{
    TP *tp = m_tpmgr->getItem(idx);
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
            NMessageBox *msg = new NMessageBox(QMessageBox::Information,
                                              "comment",
                                              tp->data(TP::cols::comment).toString());
            msg->show();
        }
    }
}

void ThroughputView::onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

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

void ThroughputView::initThroughputChart()
{
    // throughput chart
    m_vLegendScrollBar = new QScrollBar(Qt::Vertical, this);
    m_tpplot=new TPPlot(m_showgroup, m_tpunit, ui->widget_console);
    qDebug() << "enable openGl:" << m_tpplot->openGl();
    m_tpplot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tpplot, &TPPlot::customContextMenuRequested, this, &ThroughputView::onPlotContextMenuRequest);
    connect(m_tpplot, &TPPlot::selectedTPitem, this, &ThroughputView::onSelectedTPitem);
    connect(m_tpplot, &TPPlot::sigLegendCount, this ,&ThroughputView::onVLegendScrollBarRange);
    connect(this, &ThroughputView::updateInterval, m_tpplot, &TPPlot::setInterval);
    ui->hl_console->addWidget(m_tpplot);
    ui->hl_console->addWidget(m_vLegendScrollBar);
    connect(m_vLegendScrollBar, &QScrollBar::valueChanged, m_tpplot, &TPPlot::onVLegendScrollChanged);
    // m_vLegendScrollBar->setRange(0, m_tpplot->legend->itemCount() - 10);
    onVLegendScrollBarRange(m_tpplot->legend->itemCount());

    m_tpmgr = new TPMgr(m_showgroup, ui->tv_throughput, m_tpunit);
    // connect(m_tpmgr, &TPMgr::rowsInserted, this, &ThroughputView::onTPDataUpdate);
    // connect(m_tpmgr, &TPMgr::rowsRemoved, this, &ThroughputView::onTPDataUpdate);
    connect(m_tpmgr, &TPMgr::IperfTPdata, m_tpplot, &TPPlot::onIperfTPdata);

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

    TooltipEventFilter* filter = new TooltipEventFilter(ui->tv_throughput);
    connect(filter, &TooltipEventFilter::doCopy, this, &ThroughputView::onCopy);
    connect(filter, &TooltipEventFilter::doPaste, this, &ThroughputView::onPaste);
    connect(filter, &TooltipEventFilter::doDelete, this, &ThroughputView::onDelete);
    ui->tv_throughput->viewport()->installEventFilter(filter);
    ui->tv_throughput->setRootIsDecorated(true); //show folding icon
    // ui->tv_throughput->setRootIndex(m_tpmgr->getRootItemIdx()); //enable this will cause total/group item disappear!!
    //    ui->tv_throughput->expand(m_tpmgr->getRootItemIdx());

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

    QItemSelectionModel *ism = ui->tv_throughput->selectionModel();
    connect(ism, &QItemSelectionModel::selectionChanged, this, &ThroughputView::onTPselectionChanged);

}
