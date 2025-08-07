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

    //link line
    mLinkLineLayer = new QGVLayer();
    mMap->addItem(mLinkLineLayer);
    // Marker Layer;
    mItemsLayer = new QGVLayer();
    mMap->addItem(mItemsLayer);
    // poly Layer
    mPolysLayer = new QGVLayer();
    mMap->addItem(mPolysLayer);

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
    connect(ui->cbShowLinkLine, &QCheckBox::clicked, this, &DlgGeoOSM::showLinkline);
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

void DlgGeoOSM::addLinkline(QGV::GeoPos pos1, QGV::GeoPos pos2, QColor color, qreal linewidth)
{
    QVector<QGV::GeoPos> linePoints{pos1, pos2};
    addPolylines(linePoints, color, linewidth);
}

void DlgGeoOSM::showLinkline(bool show)
{
    for (int i=0; i<mLinkLineLayer->countItems();i++){
        QGVItem *itm = mLinkLineLayer->getItem(i);
        itm->setVisible(show);
    }
}

void DlgGeoOSM::addRectangle(QGV::GeoPos pos1, QPointF size, QColor color,
                             QString label)
{
    auto base = mMap->getProjection()->geoToProj(pos1);
    // qDebug() << "addRectangle: base:" << base;
    QGV::GeoRect pos = mMap->getProjection()->projToGeo({ base, base + QPointF(size.x(), size.y()) });
    // TODO: the Rectangle should consider size, and place the pos at center of Rectangle
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

void DlgGeoOSM::clearAllPlot()
{
    clearMarker();
    clearPolyLines();
    clearLinkLines();
}

void DlgGeoOSM::setItmHighlight(QString label)
{
    for(int i=0;i<mItemsLayer->countItems();i++)
    {
        RectangleText *itm = static_cast<RectangleText*>(mItemsLayer->getItem(i));
        if (label.compare(itm->getText())==0){
            itm->setSelected(true); // show item selected
        }else{
            itm->setSelected(false);

        }
    }
    qDebug() << "mPolysLayer:num:" << mPolysLayer->countItems();
    for(int i=0;i<mPolysLayer->countItems();i++)
    {
        QGVItem *itm = mPolysLayer->getItem(i);
        qDebug() << "mPolysLayer:" << itm;
    }
}

QPixmap DlgGeoOSM::createQGVImage() const
{

    const auto target = QSize(150, 150);
    // create a TextPath of "QGeoView"
    const auto path = QGV::createTextPath(QRect(QPoint(0, 0), target), "QGeoView", QFont(), 1);
    QImage image(target, QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgba(0, 0, 0, 0));
    QPixmap pixmap = QPixmap::fromImage(image, Qt::NoFormatConversion);
    QPainter painter(&pixmap);
    QPen pen = QPen(Qt::black);
    pen.setWidth(1);
    pen.setCosmetic(true);
    painter.setPen(pen);
    QBrush brush = QBrush(Qt::black);
    painter.setBrush(brush);
    painter.drawPath(path);
    return pixmap;
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

    QVector<QGV::GeoPos> linePoints{pos1, pos2};
    addPolylines(linePoints, Qt::GlobalColor::red, 5);
}

void DlgGeoOSM::onAddArrowLine(bool checked)
{
    Q_UNUSED(checked)
    QGV::GeoPos org= QGV::GeoPos{ui->ArrowLatitude->value(),
                                  ui->ArrowLongitude->value()};
    addArrowLine(org, ui->ArrowAzimuth->value(), ui->ArrowLength->value(),
                 QColor(Qt::red), ui->ArrowLineWidth->value());
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
    QGV::GeoPos pos = mfrmAddRect->getPos();
    QString label = mfrmAddRect->getLable();
    QSize size = mfrmAddRect->getSize();
    QColor c= mfrmAddRect->getColor();
    addRectangle(pos, QPointF(size.width(), size.height()), c, label);
    emit addPosition(label, pos.latitude(), pos.longitude());
}

void DlgGeoOSM::createContextMenu()
{
    QAction *actAddPosition = new QAction("Add Position", this);
    connect(actAddPosition, &QAction::triggered, this, &DlgGeoOSM::onAddPosition);

    QAction *actPosition = new QAction("Copy current mouse position", this);
    connect(actPosition, &QAction::triggered, this, &DlgGeoOSM::onCopyMousePosition);
    mMap->addAction(actAddPosition);

    mMap->addAction(actPosition);

}

void DlgGeoOSM::createTrackingWidget()
{
    // QGVWidgetText will be used to show current position.
    QGVWidgetText* text = new QGVWidgetText();
    // text->setAnchor(QPoint(0, 0), { Qt::TopEdge });
    text->setAnchor(QPoint(0, 0), { Qt::BottomEdge });
    mMap->addWidget(text);
    connect(mMap, &QGVMap::mapMouseMove, text, [this, text](QPointF projPos) {
        // Current projection position can be converted to geo-coordinates and printed by corresponding functions.
        QGV::GeoPos geoPos = mMap->getProjection()->projToGeo(projPos);
        currentMousePos->setLat(geoPos.latitude());
        currentMousePos->setLon(geoPos.longitude());
        text->setText(QString("<b>%1, %2</b>").arg(geoPos.latToString(), geoPos.lonToString()));
    });
}

void DlgGeoOSM::addPolylines(const QVector<QGV::GeoPos> &linePts, QColor color, qreal linewidth)
{
    mLinkLineLayer->addItem(new Polyline(linePts, color, linewidth));
}

void DlgGeoOSM::onAddPosition(bool checked)
{
    Q_UNUSED(checked)
    // add a device at current mouse pos
    mfrmAddRect->setPos(currentMousePos->latitude(), currentMousePos->longitude());
    mfrmAddRect->show();
}

void DlgGeoOSM::onCopyMousePosition(bool checked)
{
    Q_UNUSED(checked)
    QString s= currentMousePos->latToString() + "," + currentMousePos->lonToString();
    clipboard->setText(s);
}

void DlgGeoOSM::addArrowLine(QGV::GeoPos origin, double azimuthDeg, double length,
                             QColor color, qreal linewidth,
                             double arrowLength, double arrowAngleDeg)
{
    DirectionArrow *arrowline = new DirectionArrow(origin, azimuthDeg, length,
                                                   color, linewidth,
                                                   arrowLength, arrowAngleDeg);
    mPolysLayer->addItem(arrowline);
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
