#ifndef FORMQIPERFDS_H
#define FORMQIPERFDS_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace Ui {
class FormQIperfds;
}
// GUI to show QIperfd list
class FormQIperfds : public QWidget
{
    Q_OBJECT

public:
    explicit FormQIperfds(QWidget *parent = nullptr);
    ~FormQIperfds();
    void setModel(QAbstractItemModel *model);
    void setColumnWidth(int column, int width);
protected:
    void changeEvent(QEvent *e);
private slots:
    void onContextMenu(QPoint pos);
    void onRestart(bool checked);
    void askRestart(QString target);
private:
    void initMenu();
    Ui::FormQIperfds *ui;
    QSortFilterProxyModel *proxyModel;
    QMenu *m_menu;
    QAction *m_restartAction;
};

#endif // FORMQIPERFDS_H
