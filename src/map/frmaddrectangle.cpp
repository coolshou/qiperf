#include "frmaddrectangle.h"
#include "ui_frmaddrectangle.h"

#include <QDebug>

FrmAddRectangle::FrmAddRectangle(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FrmAddRectangle)
{
    ui->setupUi(this);
    ui->cbColor->setItemDelegate(new ColorDelegate(ui->cbColor));
    ui->cbColor->addItem("Red", QColor(Qt::red));
    ui->cbColor->addItem("Yellow", QColor(Qt::yellow));
    ui->cbColor->addItem("Blue", QColor(Qt::blue));
    ui->cbColor->addItem("Green", QColor(Qt::green));
    ui->cbColor->addItem("Cyan", QColor(Qt::cyan));
    ui->cbColor->addItem("Gray", QColor(Qt::gray));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FrmAddRectangle::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FrmAddRectangle::close);
}

FrmAddRectangle::~FrmAddRectangle()
{
    delete ui;
}

void FrmAddRectangle::setPos(double lat, double lon)
{
    mlatitide = lat;
    mlontitude = lon;
}

QGV::GeoPos FrmAddRectangle::getPos()
{
    QGV::GeoPos pos;
    pos.setLat(mlatitide);
    pos.setLon(mlontitude);
    return pos;
}

QString FrmAddRectangle::getLable()
{
    return ui->leLabel->text();
}

QSize FrmAddRectangle::getSize()
{
    return QSize(ui->sbSizeWidth->value(), ui->sbSizeHight->value());
}

QColor FrmAddRectangle::getColor()
{
    QColor selectedColor = ui->cbColor->currentData(Qt::UserRole).value<QColor>();
    return selectedColor;
}

void FrmAddRectangle::onAccepted()
{
    emit accepted();
    close();
}

void FrmAddRectangle::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
