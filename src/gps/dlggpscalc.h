#ifndef DLGGPSCALC_H
#define DLGGPSCALC_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include "../map/dlgopenstreetmap.h"

namespace Ui {
class DlgGpsCalc;
}

class DlgGpsCalc : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGpsCalc(QWidget *parent = nullptr);
    ~DlgGpsCalc();

protected:
    void changeEvent(QEvent *e);

private slots:
    void initAction();
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onClear(bool checked);
    void onTaipei101SkyTree(bool checked);
    void onCalcCliecked(bool checked);
    void onShowMap(bool checked);

    void showContextMenu(const QPoint &pos);
private:
    Ui::DlgGpsCalc *ui;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
    DlgOpenStreetMap *m_dlgOSM;
};

#endif // DLGGPSCALC_H
