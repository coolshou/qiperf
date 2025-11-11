#include "dlggeoosm.h"
#include "ui_dlggeoosm.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QApplication>
#include <QAction>

#include "../lib/geoview/polyline.h"
#include "../lib/geoview/directionarrow.h"
#include "../lib/geoview/rectangletext.h"

#include <QGeoView/QGVWidgetText.h>
#include "../lib/qgeoview/samples/shared/rectangle.h"
#include <QDebug>

DlgGeoOSM::DlgGeoOSM(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DlgGeoOSM)
{
    ui->setupUi(this);
    currentMousePos = new QGV::GeoPos();
    clipboard = QApplication::clipboard();
    mfrmAddRect = new FrmAddRectangle(this);
    connect(mfrmAddRect, &FrmAddRectangle::accepted, this, &DlgGeoOSM::onAddRectangleAccepted);
    Helpers::setupCachedNetworkAccessManager(this);

    mMap = new QGVMap(this);

    // connect(mMap, &QGVMap::scaleChanged, this , &DlgGeoOSM::onScaleChanged);
    // connect(mMap, &QGVMap::stateChanged, this, &DlgGeoOSM::onMapStateChanged);
    // Background layer
    auto osmLayer = new QGVLayerOSM();
    mMap->addItem(osmLayer);

    mBeamLayer = new QGVLayer();
    mMap->addItem(mBeamLayer);
    //link line
    mLinkLineLayer = new QGVLayer();
    mMap->addItem(mLinkLineLayer);
    // Marker Layer;
    mItemsLayer = new QGVLayer();
    mMap->addItem(mItemsLayer);
    // poly Layer
    mPolysLayer = new QGVLayer();
    mMap->addItem(mPolysLayer);
    // init poly Layer
    mInitLayer = new QGVLayer();
    mMap->addItem(mInitLayer);

    ui->vlGeo->addWidget(mMap);
    // Options list
    createOptionsList();

    createContextMenu();
    mMap->setMouseAction(QGV::MouseAction::ContextMenu, true); //custom right mouse ContextMenu
    mMap->setMouseAction(QGV::MouseAction::Tooltip, true);
    mMap->setMouseTracking(true);

    createTrackingWidget();
    // ui->vlGeo->addWidget(createOptionsList());
    connect(ui->pbSetCenter, &QPushButton::clicked, this, &DlgGeoOSM::onSetCenter);
    connect(ui->pbAddMark, &QPushButton::clicked, this, &DlgGeoOSM::onAddMark);
    connect(ui->pbClearMark, &QPushButton::clicked, this, &DlgGeoOSM::onClearMark);
    connect(ui->pbAddPolyline, &QPushButton::clicked, this, &DlgGeoOSM::onAddPolylines);
    connect(ui->pbAddArrowLine, &QPushButton::clicked, this, &DlgGeoOSM::onAddArrowLine);
    connect(ui->pbAddBeam, &QPushButton::clicked, this, &DlgGeoOSM::onAddBeam);
    connect(ui->pbClearBeam, &QPushButton::clicked, this, &DlgGeoOSM::onClearBeam);
    connect(ui->cbShowLinkLine, &QCheckBox::clicked, this, &DlgGeoOSM::showLinkline);
    connect(ui->cbShowHeadingLine, &QCheckBox::clicked, this, &DlgGeoOSM::showHeadingLine);
    connect(ui->cbShowInitHeadingLine, &QCheckBox::clicked, this, &DlgGeoOSM::showInitHeadingLine);
}

DlgGeoOSM::~DlgGeoOSM()
{
    delete ui;
}

void DlgGeoOSM::load(QString tile, double lat, double lon)
{
    m_tile = tile;
    setWindowTitle(m_tile);
    // map center is lat, lon
    double lat1 = lat + 0.02245;
    double lon1 = lon - 0.0248;
    double lat2 = lat - 0.02245;
    double lon2 = lon + 0.0248;
    load(lat1, lon1, lat2, lon2);
}

void DlgGeoOSM::load(double lat1, double lon1, double lat2, double lon2)
{
    // qDebug() << " (lat1,lon1)=" << QString::number(lat1, 'f', 6) << " , " << QString::number(lon1, 'f', 6)
    //          << " (lat2,lon2)=" << QString::number(lat2, 'f', 6) << " , " << QString::number(lon2, 'f', 6);

    auto target = QGV::GeoRect(QGV::GeoPos(lat1, lon1), QGV::GeoPos(lat2, lon2));
    mMap->cameraTo(QGVCameraActions(mMap).scaleTo(target));
    // 200m
    mMap->cameraTo(QGVCameraActions(mMap).scaleTo(0.55));
    emit loadFinished(true);
}

void DlgGeoOSM::addMarker(double lat, double lon, QString label,
                          Placemark::MarkColor color)
{
    Q_UNUSED(label)
    QGV::GeoPos geoPos = QGV::GeoPos(lat, lon);
    // rectangle

    // Placemark
    auto image = new Placemark(geoPos, color);
    mItemsLayer->addItem(image);

    QGVItem* item = mItemsLayer->getItem(mItemsLayer->countItems() - 1);
    item->setSelectable(true);
}

void DlgGeoOSM::addLinkline(QGV::GeoPos pos1, QGV::GeoPos pos2, QColor color,
                            qreal linewidth, QString label)
{
    QVector<QGV::GeoPos> linePoints{pos1, pos2};
    addPolylines(linePoints, color, linewidth, label);
}

void DlgGeoOSM::showLinkline(bool show)
{
    for (int i=0; i<mLinkLineLayer->countItems();i++){
        QGVItem *itm = mLinkLineLayer->getItem(i);
        itm->setVisible(show);
    }
}

void DlgGeoOSM::showHeadingLine(bool show)
{
    for (int i=0; i<mPolysLayer->countItems();i++){
        QGVItem *itm = mPolysLayer->getItem(i);
        itm->setVisible(show);
    }
}

void DlgGeoOSM::showInitHeadingLine(bool show)
{
    for (int i=0; i<mInitLayer->countItems();i++){
        QGVItem *itm = mInitLayer->getItem(i);
        itm->setVisible(show);
    }
}

void DlgGeoOSM::addRectangle(QGV::GeoPos pos1, QPointF size, QColor color,
                             QString label)
{
    auto base = mMap->getProjection()->geoToProj(pos1);
    // qDebug() << "addRectangle: base:" << base;
    double x = size.x()/2;
    double y = size.y()/2;
    QGV::GeoRect pos = mMap->getProjection()->projToGeo({base-QPointF(x, y),
                                                         base + QPointF(x, y)});

    // TODO: the Rectangle should consider size
    RectangleText *item = new RectangleText(label, pos, size, color, mMap);
    item->setFlag(QGV::ItemFlag::Highlightable, true);
    item->setSelectable(true);
    // item->setFlag(QGV::ItemFlag::Transformed, true);
    // mPolysLayer->addItem(item);
    mItemsLayer->addItem(item);
}

void DlgGeoOSM::clearMarker()
{
    mItemsLayer->deleteItems();
}

void DlgGeoOSM::clearPolyLines()
{
    mPolysLayer->deleteItems();
}

void DlgGeoOSM::clearLinkLines()
{
    mLinkLineLayer->deleteItems();
}

void DlgGeoOSM::clearInitLines()
{
    mInitLayer->deleteItems();
}

void DlgGeoOSM::clearAllPlot()
{
    clearMarker();
    clearPolyLines();
    clearLinkLines();
    clearInitLines();
}

void DlgGeoOSM::setItmHighlight(QString label)
{
    for(int i=0;i<mItemsLayer->countItems();i++)
    {
        RectangleText *itm = static_cast<RectangleText*>(mItemsLayer->getItem(i));
        if(itm){
            if (label.compare(itm->getText())==0){
                itm->setSelected(true); // show item selected
            }else{
                itm->setSelected(false);
            }
        }
    }
    // for(int i=0;i<mPolysLayer->countItems();i++)
    // {
    //     QGVItem *itm = mPolysLayer->getItem(i);
    //     qDebug() << "TODO:[setItmHighlight] mPolysLayer itm:" << itm;
    // }
}

void DlgGeoOSM::onDeleteItm(QString label)
{
    for(int i=0;i<mItemsLayer->countItems();i++)
    {
        RectangleText *itm = static_cast<RectangleText*>(mItemsLayer->getItem(i));
        if(itm){
            if (label.compare(itm->getText())==0){
                mItemsLayer->removeItem(itm);
            }
        }
    }
    // also remove relative arrow
    for(int i=0;i<mInitLayer->countItems();i++)
    {
        DirectionArrow *itm = static_cast<DirectionArrow*>(mInitLayer->getItem(i));
        if(itm){
            if (label.compare(itm->getLabel())==0){
                mInitLayer->removeItem(itm);
            }
        }
    }
    for(int i=0;i<mPolysLayer->countItems();i++)
    {
        DirectionArrow *itm = static_cast<DirectionArrow*>(mPolysLayer->getItem(i));
        if(itm){
            if (label.compare(itm->getLabel())==0){
                mPolysLayer->removeItem(itm);
            }
        }
    }
    // TODO: also remove relative LinkLine
    for(int i=0;i<mLinkLineLayer->countItems();i++){
        Polyline *itm = static_cast<Polyline*>(mLinkLineLayer->getItem(i));
        if(itm){
            if (label.compare(itm->getLabel())==0){
                mLinkLineLayer->removeItem(itm);
            }
        }
    }
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

void DlgGeoOSM::onAddMark(bool checked)
{
    Q_UNUSED(checked)
    addMarker(ui->sbMarkLat->value(), ui->sbMarkLon->value());
}

void DlgGeoOSM::onClearMark(bool checked)
{
    Q_UNUSED(checked)
    clearMarker();
}

void DlgGeoOSM::onAddPolylines(bool checked)
{
    Q_UNUSED(checked)
    // test data
    QGV::GeoPos pos1{ui->sbLat->value(), ui->sbLon->value()};
    QGV::GeoPos pos2{ui->sbMarkLat->value(), ui->sbMarkLon->value()};

    addLinkline(pos1, pos2, Qt::GlobalColor::red, 5);
}

void DlgGeoOSM::onAddArrowLine(bool checked)
{
    Q_UNUSED(checked)
    QGV::GeoPos org= QGV::GeoPos{ui->ArrowLatitude->value(),
                                  ui->ArrowLongitude->value()};
    addArrowLine(org, ui->ArrowAzimuth->value(), ui->ArrowLength->value(),
                 QColor(Qt::red), ui->ArrowLineWidth->value());
}

void DlgGeoOSM::onAddBeam(bool checked)
{
    Q_UNUSED(checked)
    QGV::GeoPos org= QGV::GeoPos{ui->BeamLatitude->value(),
                                  ui->BeamLongitude->value()};
    addBeamItem(org, ui->sbAzDegree->value(), ui->sbHPAz->value(),
                ui->sbRange->value(), QColor(ui->beamColor->currentText()));
}

void DlgGeoOSM::onClearBeam(bool checked)
{
    Q_UNUSED(checked)
    mBeamLayer->deleteItems();
}

void DlgGeoOSM::onMapStateChanged(QGV::MapState state)
{
    qDebug() << "onMapStateChanged:" << QString::number(static_cast<int>(state));
}

void DlgGeoOSM::onScaleChanged()
{
    // TODO: when map change scale
    // qDebug() <<  "onScaleChanged:" << mMap->getCamera().scale();

    /*
    QPointF top_left_geo = mMap->mapToProj(QPoint(0,0));
    QPointF bottom_right_geo = mMap->mapToProj(QPoint(mMap->width(), mMap->height()));

    qDebug() << "onScaleChanged:" << QString::number(top_left_geo.x())
             << ", " << QString::number(top_left_geo.y())
             << " bottom_right_geo:" << QString::number(bottom_right_geo.x())
             << ", " << QString::number(bottom_right_geo.y());
    */
}

void DlgGeoOSM::onAddRectangleAccepted()
{
    bool editmode = mfrmAddRect->getEditMode();
    QGV::GeoPos pos = mfrmAddRect->getPos();
    QString label = mfrmAddRect->getLable();
    QSize size = mfrmAddRect->getSize();
    QColor c= mfrmAddRect->getColor();
    if (editmode){
        qDebug() << "TODO: edit current Rectangle";
    }else{
        addRectangle(pos, QPointF(size.width(), size.height()), c, label);
        emit addPosition(label, pos.latitude(), pos.longitude());
    }
}

void DlgGeoOSM::createContextMenu()
{
    actAddPosition = new QAction("Add Position", this);
    connect(actAddPosition, &QAction::triggered, this, &DlgGeoOSM::onAddPosition);

    actEditPosition = new QAction("Edit Position", this);
    // actEditPosition->setEnabled(false);
    actEditPosition->setVisible(false);
    connect(actEditPosition, &QAction::triggered, this, &DlgGeoOSM::onEditPosition);

    actPosition = new QAction("Copy current mouse position", this);
    connect(actPosition, &QAction::triggered, this, &DlgGeoOSM::onCopyMousePosition);

    mMap->addAction(actAddPosition);
    mMap->addAction(actEditPosition);
    mMap->addAction(actPosition);

}

void DlgGeoOSM::createTrackingWidget()
{
    // QGVWidgetText will be used to show current GPS position .
    QGVWidgetText* text = new QGVWidgetText();
    QSet<Qt::Edge> combinedEdge;//= Qt::RightEdge | Qt::BottomEdge;
    combinedEdge.insert(Qt::RightEdge);
    combinedEdge.insert(Qt::BottomEdge);
    text->setAnchor(QPoint(5, -2),  combinedEdge);
    mMap->addWidget(text);
    connect(mMap, &QGVMap::mapMouseMove, text, [this, text](QPointF projPos) {
        // Current projection position can be converted to geo-coordinates and printed by corresponding functions.
        QGV::GeoPos geoPos = mMap->getProjection()->projToGeo(projPos);
        currentMousePos->setLat(geoPos.latitude());
        currentMousePos->setLon(geoPos.longitude());
        text->setText(QString("<b>%1, %2</b>").arg(geoPos.latToString(), geoPos.lonToString()));
    });
}

void DlgGeoOSM::addPolylines(const QVector<QGV::GeoPos> &linePts, QColor color,
                             qreal linewidth, QString label)
{
    Polyline *itm = new Polyline(linePts, color, linewidth, label);
    mLinkLineLayer->addItem(itm);
}

void DlgGeoOSM::onAddPosition(bool checked)
{
    Q_UNUSED(checked)
    // add a device at current mouse pos
    mfrmAddRect->setEditMode(false);
    mfrmAddRect->setPos(currentMousePos->latitude(), currentMousePos->longitude());
    mfrmAddRect->show();
}

void DlgGeoOSM::onEditPosition(bool checked)
{
    Q_UNUSED(checked)
    QString label="";
    QColor  color=QColor(Qt::red);
    QPointF size= QPointF(10,10);
    // QGV::GeoRect pos;
    // mItemsLayer;
    for(int i=0;i<mItemsLayer->countItems();i++)
    {
        RectangleText *itm = static_cast<RectangleText*>(mItemsLayer->getItem(i));
        if(itm){
            if (itm->isSelected()){
                label = itm->getText();
                color = itm->getColor();
                size = itm->getSize();
                // pos = itm->getPos();
                break;
            }
        }
    }
    mfrmAddRect->setEditMode(true);
    mfrmAddRect->setLabel(label);
    mfrmAddRect->setColor(color);
    mfrmAddRect->setSize(size);
    // mfrmAddRect->setPos(pos.);
    mfrmAddRect->show();
}

void DlgGeoOSM::onCopyMousePosition(bool checked)
{
    Q_UNUSED(checked)
    QString s= currentMousePos->latToString() + "," + currentMousePos->lonToString();
    clipboard->setText(s);
}

void DlgGeoOSM::addArrowLine(QGV::GeoPos origin, double azimuthDeg, double length,
                             QColor color, bool init, qreal linewidth,
                             double arrowLength, double arrowAngleDeg,
                             QString lable)
{
    DirectionArrow *arrowline = new DirectionArrow(origin, azimuthDeg, length,
                                                   color, linewidth,
                                                   arrowLength, arrowAngleDeg,
                                                   lable);
    if (init){
        mInitLayer->addItem(arrowline);
    }else{
        mPolysLayer->addItem(arrowline);
    }

}

QGroupBox* DlgGeoOSM::createOptionsList(bool addCheckbox)
{
    QList<QPair<QString, QGVWidget*>> widgets = {
                                                 {"Compass", new QGVWidgetCompass()},
                                                 {"ScaleHorizontal", new QGVWidgetScale(Qt::Horizontal)},
                                                 {"ScaleVertical", new QGVWidgetScale(Qt::Vertical)},
                                                };
    //{"ZoomButtons", new QGVWidgetZoom()},
    QGroupBox* groupBox = nullptr;
    if (addCheckbox){
        groupBox = new QGroupBox(tr("Map widgets"));
        groupBox->setLayout(new QVBoxLayout);
    }
    for (auto pair : widgets) {
        auto name = pair.first;
        auto widget = pair.second;

        mMap->addWidget(widget);
        if (addCheckbox){
            QCheckBox* checkButton = new QCheckBox(name);
            checkButton->setChecked(true);
            connect(checkButton, &QCheckBox::clicked, this, [widget](const bool checked) { widget->setVisible(checked); });
            if (groupBox){
                groupBox->layout()->addWidget(checkButton);
            }
        }
    }
    if (addCheckbox){
        if (groupBox){
            return groupBox;
        }
    }
    return nullptr;

}

void DlgGeoOSM::addBeamItem(QGV::GeoPos origin, double azimuthDeg, double hpbwDeg, double rangeMeters, const QColor &color)
{
    BeamItem* beam1 = new BeamItem(origin, azimuthDeg, hpbwDeg, rangeMeters, color);
    mBeamLayer->addItem(beam1);
}
