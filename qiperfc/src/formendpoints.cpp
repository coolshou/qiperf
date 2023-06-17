#include "formendpoints.h"
#include "ui_formendpoints.h"

FormEndPoints::FormEndPoints(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormEndPoints)
{
    ui->setupUi(this);
}

FormEndPoints::~FormEndPoints()
{
    delete ui;
}
