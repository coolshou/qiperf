#ifndef DLGGEOOSM_H
#define DLGGEOOSM_H

/*
 Use QGeoView to display Geographic Information of OSM (OpenStreetMap)

show:
    1. place marker
    2. polyline
    3. rf sector (fan-shaped) Coverage
*/
#include <QDialog>
#include <QGroupBox>
#include <QClipboard>

#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>
#include <QGeoView/QGVGlobal.h>
#include <helpers.h>
#include "../lib/geoview/placemark.h"
#include "frmaddrectangle.h"
#include "beamitem.h"

namespace Ui {
class DlgGeoOSM;
}

class DlgGeoOSM : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGeoOSM(QWidget *parent = nullptr);
    ~DlgGeoOSM() override;
    void load(QString tile, double lat=24.804162, double lon=121.027736);
    void load(double lat1, double lon1, double lat2, double lon2);
    void addMarker(double lat, double lon, QString label="lable",
                   Placemark::MarkColor color=Placemark::MarkColor::Yellow);
    void addLinkline(QGV::GeoPos pos1, QGV::GeoPos pos2,
                     QColor color=Qt::GlobalColor::yellow, qreal linewidth=5,
                     QString label="");
    void addRectangle(QGV::GeoPos pos1, QPointF size=QPointF(20.0, 10.0),
                      QColor color=Qt::GlobalColor::yellow, QString label="");
    void addArrowLine(QGV::GeoPos origin, double azimuthDeg, double length,
                      QColor color=QColor(Qt::red), bool init=false,
                      qreal linewidth=3, double arrowLength = 20.0,
                      double arrowAngleDeg = 30.0,
                      QString lable="");
    void addBeamItem(QGV::GeoPos origin, double azimuthDeg, double hpbwDeg,
                     double rangeMeters, const QColor& color);
    void clearMarker();
    void clearPolyLines();
    void clearLinkLines();
    void clearInitLines();
    void clearAllPlot();

public slots:
    void setItmHighlight(QString label);
    void onDeleteItm(QString label);

protected slots:
    void onSetCenter(bool checked);
    void onAddMark(bool checked);
    void onClearMark(bool checked);
    void onAddPolylines(bool checked);
    void showLinkline(bool show);
    void showHeadingLine(bool show);
    void showInitHeadingLine(bool show);
    void onAddArrowLine(bool checked);
    void onAddBeam(bool checked);
    void onClearBeam(bool checked);
    void onMapStateChanged(QGV::MapState state);
    void onScaleChanged();
    void onAddRectangleAccepted();
signals:
    void loadFinished(bool ok);
    void addPosition(QString label, double lat, double lon);
private:
    void createContextMenu();
    void createTrackingWidget();
    void addPolylines(const QVector<QGV::GeoPos>& linePts, QColor color,
                     qreal linewidth=1, QString label="");
    void onAddPosition(bool checked);
    void onEditPosition(bool checked);
    void onCopyMousePosition(bool checked);
    QGroupBox* createOptionsList(bool addCheckbox=false);

    Ui::DlgGeoOSM *ui;
    QGVMap *mMap;
    QGVLayer* mBeamLayer;
    QGVLayer* mLinkLineLayer;
    QGVLayer* mItemsLayer;
    QGVLayer* mPolysLayer;
    QGVLayer* mInitLayer;
    QString m_tile;
    QGV::GeoPos *currentMousePos;
    QClipboard *clipboard;
    FrmAddRectangle *mfrmAddRect;
    QAction *actAddPosition;
    QAction *actEditPosition;
    QAction *actPosition;
};

#endif // DLGGEOOSM_H
