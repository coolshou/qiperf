#ifndef DLGGPSCALC_H
#define DLGGPSCALC_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QNetworkReply>

#include "../map/dlgopenstreetmap.h"

namespace Ui {
class DlgGpsCalc;
}

class DlgGpsCalc : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGpsCalc(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgGpsCalc();
    void isTileAvailable();
    QString getTile();
signals:
    void TileAvailable(bool ok);
    void closeAll();
protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event) override;

private slots:
    void initAction();
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onClear(bool checked);
    void onTaipei101SkyTree(bool checked);
    void onCalcCliecked(bool checked);
    void onShowMap(bool checked);
    void onToDMS(bool checked);
    void onToDegree(bool checked);
    void showContextMenu(const QPoint &pos);
    void onLoadFinished(bool ok);
    void onTileAvailable(bool ok);
    void onCheckTileFinished();
    void onCheckTileErrorOccurred(QNetworkReply::NetworkError errorcode);
private:
    Ui::DlgGpsCalc *ui;
    QSettings *m_cfg;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
    DlgOpenStreetMap *m_dlgOSM;
    QNetworkReply *reply = nullptr;
    bool showline=false;
};

#endif // DLGGPSCALC_H
