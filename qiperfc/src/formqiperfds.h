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
signals:
    void clearNtpStatus(QString target);

protected:
    void changeEvent(QEvent *e);
private slots:
    void onContextMenu(QPoint pos);
    void onRestart(bool checked);
    void onResetNtp(bool checked);
    void askRestart(QString target);
private:
    void initMenu();
    Ui::FormQIperfds *ui;
    QSortFilterProxyModel *proxyModel;
    QMenu *m_menu;
    QAction *m_restartAction;
    QAction *m_resetNtpAction;
};

#endif // FORMQIPERFDS_H
