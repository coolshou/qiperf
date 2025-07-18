#ifndef DLGGEOOSM_H
#define DLGGEOOSM_H

#include <QDialog>
#include <QGroupBox>

#include <QGeoView/QGVLayerOSM.h>
#include <QGeoView/QGVWidgetCompass.h>
#include <QGeoView/QGVWidgetScale.h>
#include <QGeoView/QGVWidgetZoom.h>
#include <helpers.h>

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
protected slots:
    void onSetCenter(bool checked);

private:
    QGroupBox* createOptionsList();

    Ui::DlgGeoOSM *ui;
    QGVMap *mMap;
    QString m_tile;
};

#endif // DLGGEOOSM_H
