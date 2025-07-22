#ifndef DLGGPSCALC_H
#define DLGGPSCALC_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QNetworkReply>

#include "../map/dlgopenstreetmap.h"
#include "../map/dlggeoosm.h"

namespace Ui {
class DlgGpsCalc;
}

class DlgGpsCalc : public QDialog
{
    Q_OBJECT

public:
    enum GPScols{
        PositionName=0,
        Latitude=1,
        Longitude=2,
        Altitude=3,
        AIP1=4,
        AIP2=5
    };
    Q_ENUM(GPScols)
    enum AZEIcols{
        Name=0,
        Distance=1,
        Azimuth1=2,
        Azimuth2=3,
        Elevation1=4,
        Elevation2=5
    };
    Q_ENUM(AZEIcols)

    explicit DlgGpsCalc(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgGpsCalc() override;
    void isTileAvailable();
    QString getTile();
    void setShowLine(bool show);
    void clearData();
signals:
    void TileAvailable(bool ok);
    void closeAll();
protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void initAction();
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onAddRow(QString name, double latitude, double longitude, double altitude);
    void onClear(bool checked);
    void onLoadCliecked(bool checked);
    void onSaveCliecked(bool checked);
    void onCalcCliecked(bool checked);
    void onShowMap(bool checked);
    void onShowGeo(bool checked);
    void onShow3D(bool checked);
    void onToDMS(bool checked);
    void onToDegree(bool checked);
    void showContextMenu(const QPoint &pos);
    void onLoadFinished(bool ok);
    void onTileAvailable(bool ok);
    void onCheckTileFinished();
    void onCheckTileErrorOccurred(QNetworkReply::NetworkError errorcode);
    void handleButtonClicked(int row, int col);
private:
    void onLoad(QString filename);
    bool onSave(QString filename);
    void debug(QString msg, int lv=3);
    Ui::DlgGpsCalc *ui;
    QSettings *m_cfg;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
    DlgOpenStreetMap *m_dlgOSM;
    DlgGeoOSM *m_dlgGeo;
    QNetworkReply *reply = nullptr;
    bool showline=false;
    QString m_oldsavepath;
    int m_debuglv;
};

#endif // DLGGPSCALC_H
