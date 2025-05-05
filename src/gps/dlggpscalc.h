#ifndef DLGGPSCALC_H
#define DLGGPSCALC_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>

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
    void onPosCliecked(bool checked);

    void showContextMenu(const QPoint &pos);
private:
    Ui::DlgGpsCalc *ui;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
};

#endif // DLGGPSCALC_H
