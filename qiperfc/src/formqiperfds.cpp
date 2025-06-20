#include "formqiperfds.h"
#include "ui_formqiperfds.h"

#include <QThread>
#include <QCoreApplication>
#include <QEvent>

#include "comm.h"
#include "wsclient.h"

FormQIperfds::FormQIperfds(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQIperfds)
{
    ui->setupUi(this);
    initMenu();
    //setColumnWidth(0, 160);
    //setColumnWidth(1, 180);
    // Create the proxy model for sorting
    proxyModel = new QSortFilterProxyModel(this);
    // Ensure that the user can click on the header to sort
    ui->treeView->header()->setSortIndicatorShown(true);   // Show the sort indicator
    ui->treeView->header()->setSectionsClickable(true);    // Make headers clickable
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);  // custom right click menu
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &FormQIperfds::onContextMenu);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

FormQIperfds::~FormQIperfds()
{
    delete ui;
}

void FormQIperfds::setModel(QAbstractItemModel *model)
{
    proxyModel->setSourceModel(model);
    ui->treeView->setModel(proxyModel);
    ui->treeView->setSortingEnabled(true);
    // Optionally, initially sort by the first column
    ui->treeView->sortByColumn(0, Qt::AscendingOrder);
//    ui->treeView->setModel(model);
}

void FormQIperfds::setColumnWidth(int column, int width)
{
    ui->treeView->setColumnWidth(column, width);
}

void FormQIperfds::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FormQIperfds::onContextMenu(QPoint pos)
{
    m_menu->popup(ui->treeView->mapToGlobal(pos));
}

void FormQIperfds::onRestart(bool checked)
{
    Q_UNUSED(checked)
    QModelIndexList idxs = ui->treeView->selectionModel()->selectedRows();
    foreach (auto midx, idxs) {
        qDebug()<< "onRestart midx:" << midx;
        QString target=ui->treeView->model()->data(midx).toString();
        qDebug()<< "data:" <<target;
        askRestart(target);
    };
}

void FormQIperfds::askRestart(QString target)
{
    qDebug() << "Ask qiperfd Restart:" << target;
    QString url = "ws://"+target+":"+QString::number(QIPERFD_WSPORT);
    WSClient wsc= WSClient(target, QUrl(url), "");
    int timeout=0;
    while (!wsc.isConnected() && (timeout<30)){
        QThread::msleep(100);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        timeout++;
    }
    if (wsc.isConnected()){
        QString cmd=CMD_QIPERFD_RESTART;
        wsc.sendText(cmd);
        wsc.close();
    }else{
        qDebug() << "did not connect to " << url;
    }
}

void FormQIperfds::initMenu()
{
    m_menu=new QMenu();
    m_restartAction = new QAction("Restart qiperfd");
    connect(m_restartAction, &QAction::triggered, this, &FormQIperfds::onRestart);

    m_menu->addAction(m_restartAction);
}
