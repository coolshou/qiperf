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

#include "tpplot.h"
#include "tpmgr.h"
#include "dlgiperf.h"
#include "tpdirdelegate.h"
#include "tpfoldingdelegate.h"

#include "../views/abstractview.h"

namespace Ui {
class ThroughputView;
}

class ThroughputView : public AbstractView
{
    Q_OBJECT

public:
    // explicit ThroughputView(QIperfC *main, QWidget *parent = nullptr);
    //explicit ThroughputView(QWidget *parent = nullptr);
    explicit ThroughputView(QAction *aCopy, QAction *aPaste, QAction *aDelete,
                            QAction *aCopyText, bool showgroup=false,
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

public slots:
    void onCopy();
    void onPaste();
    void onDelete();
    void onCopyText();
    void onAddIperf();
    void onPairEdit();
    void onPairDelete();
    void onPairSwap();
    void onPairSwapIP();
    void setStartTime(QDateTime startTime);
    void onAddTPdata(QString midx, QString sInterval, QString idx,
                   QString value, QString unit, QString dir=nullptr,
                   QString pkt_lost="", QString pkt_total="");
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                         QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates);
    void onIperfTPdata(QString refrow, QString sInterval, QString datas);
    void onUpdateTPCfg(QByteArray tpcfg);
    void setShowGroup(bool bShow);
    void setXRangeUpper(double upper);
signals:
    void updateActions(bool bStart, bool bStop, bool bClear);
    void updateActionsSave(bool bStart);
    void updateActionsEdit(bool bDel, bool bEdit, bool bSwap, bool bSwapIP);
    void deleteFiles(QStringList files);

private slots:
    void initMenus();
    void onPlotContextMenuRequest(QPoint pos);
    void onSelectedTPitem(QString idx);
    void onTPUTContextMenu(QPoint pos);
    void onTPDataUpdate(const QModelIndex &parent, int first, int last);
    void aboutQCustomPlot();
    void onEnableItem(bool checked);
    void onDisableItem(bool checked);
    void onItemDClicked(QModelIndex idx);
    void onTPselectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
private:
    Ui::ThroughputView *ui;
    // QIperfC *m_main;
    TPPlot *m_tpplot;
    TPMgr *m_tpmgr;
    QClipboard *m_clipboard;
    QMenu *m_tpmenu; //right menu for m_tpmgr
    QAction *m_aEnable; //
    QAction *m_aDisable;
    QAction *m_actionCopy;
    QAction *m_actionPaste;
    QAction *m_actionDelete;
    QAction *m_actionCopyText;
    DlgIperf * dlgiperf;  // dialog of iperf config
    TPDirDelegate *tpdirdelegate;
    TPFoldingDelegate *tpfoldingdelegate;
    QDateTime m_starttime;
    bool m_showgroup;

    void initThroughputChart();
};

#endif // THROUGHPUTVIEW_H
