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

#include <QGeoView/QGVMap.h>
#include <QGeoView/QGVLayer.h>
#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>
#include <QGeoView/QGVGlobal.h>
#include <helpers.h>
#include "../lib/geoview/placemark.h"

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
    void addPolyline(QGV::GeoPos pos1, QGV::GeoPos pos2,
                     QColor color=Qt::GlobalColor::yellow, qreal linewidth=5);
    void addRectangle(QGV::GeoPos pos1, QPointF size=QPointF(10.0, 20.0),
                      QColor color=Qt::GlobalColor::yellow, QString label="");
    void clearMarker();
    void clearPolyLines();
    QPixmap createQGVImage() const;
protected slots:
    void onSetCenter(bool checked);
    void onAddMark(bool checked);
    void onClearMark(bool checked);
    void onAddPolylines(bool checked);
    void onAddArrowLine(bool checked);
    void onMapStateChanged(QGV::MapState state);
    void onScaleChanged();
signals:
    void loadFinished(bool ok);
private:
    void createContextMenu();
    void createTrackingWidget();
    void addPolylines(const QVector<QGV::GeoPos>& linePts, QColor color,
                     qreal linewidth=1);
    void addArrowLine(QGV::GeoPos origin, double azimuthDeg, double length,
                      QColor color=QColor(Qt::red), qreal linewidth=1, double arrowLength = 3.0,
                      double arrowAngleDeg = 30.0);
    QGroupBox* createOptionsList(bool addCheckbox=false);

    Ui::DlgGeoOSM *ui;
    QGVMap *mMap;
    QGVLayer* mItemsLayer;
    QGVLayer* mPolysLayer;
    QString m_tile;
};

#endif // DLGGEOOSM_H
