#ifndef DLGJIO_H
#define DLGJIO_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QNetworkReply>
#include <QJsonObject>
#include <QAbstractButton>
#include <QTimer>

// #include "../src/map/dlgopenstreetmap.h"
#include "../src/map/dlggeoosm.h"
#include "dlgaip.h"

namespace Ui {
class DlgJIO;
}

class DlgJIO : public QDialog
{
    Q_OBJECT

public:
    enum GPScols{
        PositionName=0,
        Latitude=1,
        Longitude=2,
        Altitude=3,
        Heading=4,
        Pitch=5,
        AIP1=6,
        AIP2=7,
        IPAddr=8
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
    enum AIPcols{
        Azimuth=0,
        Elevation=1
    };
    Q_ENUM(AIPcols)
    explicit DlgJIO(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgJIO() override;
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
    void onAddRow(QString name, double latitude, double longitude,
                  double altitude, double heading,
                  QJsonObject aip1=QJsonObject(), QJsonObject aip2=QJsonObject(), QString ipaddr="");
    void onClear(bool checked);
    void onLoadCliecked(bool checked);
    void onSaveCliecked(bool checked);
    void onCalcCliecked(bool checked);
    void onInquireClicked(bool checked);
    void onInquireTimerTimeout();
    // void onShowMap(bool checked);
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
    void onAcceptedAIP();
    // void onUpdateData(int row, int col, QString data);
    void onUpdateData(int row, int col, QJsonObject data);
    void onUpdateModelType(int row, int col, QString smodel);
    void onUpdateModelType(int row, int col, int model);
private:
    void onLoad(QString filename);
    bool onSave(QString filename);
    void debug(QString msg, int lv=3);
    void loadcfg();
    void savecfg();
    Ui::DlgJIO *ui;
    QSettings *m_cfg;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
    // DlgOpenStreetMap *m_dlgOSM;
    DlgGeoOSM *m_dlgGeo;
    DlgAIP *m_dlgaip;
    QNetworkReply *reply = nullptr;
    bool showline=false;
    QString m_oldsavepath;
    int m_debuglv;
    QTimer * m_InquireTimer;
};

#endif // DLGJIO_H
