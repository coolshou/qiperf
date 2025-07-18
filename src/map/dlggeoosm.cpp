#include "dlggeoosm.h"
#include "ui_dlggeoosm.h"

#include <QVBoxLayout>
#include <QCheckBox>

DlgGeoOSM::DlgGeoOSM(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgGeoOSM)
{
    ui->setupUi(this);
    Helpers::setupCachedNetworkAccessManager(this);

    mMap = new QGVMap(this);
    // Background layer
    auto osmLayer = new QGVLayerOSM();
    mMap->addItem(osmLayer);

    // TODO: Marker Layer;

    ui->vlGeo->addWidget(mMap);
    // Options list
    // ui->vlGeo->addWidget(createOptionsList());
    connect(ui->pbSetCenter, &QPushButton::clicked, this, &DlgGeoOSM::onSetCenter);
}

DlgGeoOSM::~DlgGeoOSM()
{
    delete ui;
}

void DlgGeoOSM::load(QString tile, double lat, double lon)
{
    m_tile = tile;
    // map center is lat, lon
    double lat1 = lat + 0.02245;
    double lon1 = lon + 0.0248;
    double lat2 = lat - 0.02245;
    double lon2 = lon + 0.0248;
    load(lat1, lon1, lat2, lon2);
}

void DlgGeoOSM::load(double lat1, double lon1, double lat2, double lon2)
{
    auto target = QGV::GeoRect(QGV::GeoPos(lat1, lon1), QGV::GeoPos(lat2, lon2));
    mMap->cameraTo(QGVCameraActions(mMap).scaleTo(target));
}

void DlgGeoOSM::onSetCenter(bool checked)
{
    Q_UNUSED(checked)
    if (mMap){
        double lat = ui->sbLat->value();
        double lon = ui->sbLon->value();
        load(m_tile, lat, lon);
    }
}

QGroupBox* DlgGeoOSM::createOptionsList()
{
    QList<QPair<QString, QGVWidget*>> widgets = {
                                                  { "Compass", new QGVWidgetCompass() },
                                                  { "ZoomButtons", new QGVWidgetZoom() },
                                                  { "ScaleHorizontal", new QGVWidgetScale(Qt::Horizontal) },
                                                  { "ScaleVertical", new QGVWidgetScale(Qt::Vertical) },
                                                  };

    QGroupBox* groupBox = new QGroupBox(tr("Map widgets"));
    groupBox->setLayout(new QVBoxLayout);

    for (auto pair : widgets) {
        auto name = pair.first;
        auto widget = pair.second;

        mMap->addWidget(widget);

        QCheckBox* checkButton = new QCheckBox(name);
        checkButton->setChecked(true);

        connect(checkButton, &QCheckBox::clicked, this, [widget](const bool checked) { widget->setVisible(checked); });

        groupBox->layout()->addWidget(checkButton);
    }

    return groupBox;
}
