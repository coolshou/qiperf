#ifndef THROUGHPUTVIEW_H
#define THROUGHPUTVIEW_H

#include <QWidget>
#include <QMenu>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QModelIndex>
#include <QList>
#include <QMainWindow>
#include <QItemSelection>
#include <QSettings>

#include "../src/tpplot.h"
#include "../src/tpmgr.h"
#include "../src/dlgiperf.h"
#include "../src/tpdirdelegate.h"
#include "../src/tpfoldingdelegate.h"
#include "../src/iperfwrapper.h"
#include "../src/nowrapdelegate.h"
#include "../src/tpgroup.h"

#include "abstractview.h"

namespace Ui {
class ThroughputView;
}

class ThroughputView : public AbstractView
{
    Q_OBJECT

public:
    explicit ThroughputView(QAction *aCopy, QAction *aPaste, QAction *aDelete,
                            QAction *aCopyText, QSettings *cfg,
                            int tpgroup = TPGroup::GroupMode::Detail, QString sunit="Mbps",
                            QWidget *parent = nullptr);
    ~ThroughputView() override;

    QString title() override { return tr("Throughput"); }
    QString iid() override { return "Throughput"; }
    int rootChildCount();
    QByteArray savedata();
    QStringList getPCs();
    void reset();
    QList<TP*> getChilds(bool showAll=true);
    bool addEndpoint(QString mgr, QString mdata); // mgr: manager ip, mdata: relative data
    void doClear();
    QPixmap toPixmap(int width=0, int height=0, double scale=1.0);
    void addComment(QString midx, QString comment);
    QDateTime getStartTime();
    bool getTP(QString &tpvalue, QString &lostrate); //get TP value

public slots:
    void onCopy() override;
    void onPaste() override;
    void onDelete() override;
    void onCopyText() override;

    void onAddIperf();
    void AddIperf(QString cfg);
    void onPairEdit();
    void onPairDelete();
    void onPairSwap();
    void onPairSwapIP();
    void setStartTime(QDateTime startTime);
    void onAddTPdata(QString midx, QString sInterval, QString idx,
                   QString value, QString unit, QString dir=nullptr,
                   QString pkt_lost="", QString pkt_total="");
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                         QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates,
                         int direction);
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    void onUpdateTPCfg(QByteArray tpcfg);
    void onUpdateTPUnit(QString suint);
    void onGroupCopyItem(bool checked);
    void setShowGroupTotal(bool bShow);
    void setShowGroupPair(bool bShow);
    void setShowGroupDir(bool bShow);
    void setShowGroupComment(bool bShow);
    void setTPGroupType(int grouptype);
    void getRawData(bool checked);
    void onSaveImg(bool checked);
    void setXRangeUpper(double upper);
    void setInterval(int interval);
    void onTestStarted();
    void onTestStoped(int err);
signals:
    void updateActions(bool bStart, bool bStop, bool bClear);
    void updateActionsSave(bool bStart);
    void updateActionsEdit(bool bDel, bool bEdit, bool bSwap, bool bSwapIP);
    void deleteFiles(QStringList files);
    void updateInterval(int interval);
    // void showGroup(bool bShow);
    void showGrouptype(int grouptype);
    void startstop(bool start);
private slots:
    void initMenus();
    void onPlotContextMenuRequest(QPoint pos);
    void onSelectedTPitem(QString idx);
    void onTPUTContextMenu(QPoint pos);
    void onTPDataUpdate(const QModelIndex &parent, int first, int last);
    void aboutQCustomPlot();
    void onEnableItem(bool checked);
    void onDisableItem(bool checked);
    void copyClientArgs(bool checked);
    void copyServerArgs(bool checked);
    void onItemDClicked(QModelIndex idx);
    void onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onVLegendScrollBarRange(int count);
    QString getGraphDataToJsonStr(QCPGraph *graph);
    void onDebuginfo(QString msg);
    // void onShowGrouptype(int grouptype);
private:
    Ui::ThroughputView *ui;
    TPPlot *m_tpplot;
    TPMgr *m_tpmgr;
    QClipboard *m_clipboard;
    QMenu *m_tpmenu; //right menu for m_tpmgr
    QMenu *m_tpgroupmenu;
    QAction *m_actionGroupCopyItem;
    QAction *m_aEnable; //
    QAction *m_aDisable;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionDelete;
    QAction *m_actionCopyText;
    QAction *m_actionClientArgs;
    QAction *m_actionServerArgs;
    QMenu *m_rightmenu;
    QMenu *m_menuGroup;
    QAction *m_actionGroupTotal; // group all
    QAction *m_actionGroupPair; // group by iperf test pair
    QAction *m_actionGroupDir; // group by direction
    QAction *m_actionGroupComment; // group by comment
    QAction *m_actionSaveImg;
    QAction *m_actionRawData;
    QAction *m_actionAbout;
    DlgIperf * dlgiperf;  // dialog of iperf config
    TPDirDelegate *tpdirdelegate;
    TPFoldingDelegate *tpfoldingdelegate;
    NoWrapDelegate *nowrapdelegate;
    QDateTime m_starttime;
    int m_tpgrouptype;
    QString m_tpunit;
    IperfWrapper *m_iperfwrapper;
    QScrollBar *m_vLegendScrollBar;
    QString m_oldsavepath;
    void initThroughputChart();
};

#endif // THROUGHPUTVIEW_H
