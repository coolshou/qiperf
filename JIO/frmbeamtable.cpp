#include "frmbeamtable.h"
#include "ui_frmbeamtable.h"

#include <QVBoxLayout>
#include <QFile>
#include <QBrush>

#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxworkbook.h"

#include <QDebug>

FrmBeamTable::FrmBeamTable(AIP::ModuleType moduletype, QWidget *parent):
    QCustomPlot(parent),
    ui(new Ui::FrmBeamTable), mModuletype(moduletype)
{
    ui->setupUi(this);
    setInteractions(QCP::iSelectItems | QCP::iRangeDrag | QCP::iRangeZoom);
    // axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
    // axisRect()->setRangeDragAxes(this->xAxis, this->yAxis);
    setWindowTitle("Unknown");

    setXaxis("AZ (deg)", -70, 70);
    setYaxis("EL (deg)", -30, 30);
    if (moduletype==AIP::ModuleType::Cyntec){
        setWindowTitle("Cyntec");
    }
    if (moduletype==AIP::ModuleType::Hanwha){
        setWindowTitle("Hanwha");
    }
    if (0){
        // test data
        // loadData("Cyntec_beam_table_v0.2.5.xlsx"); //test Cyntec data
        loadData("a41c_beam_table_export_v5.xlsx"); //test Hanwha data
        setGridPoints(mdata); // show points
        setEllipse(1, "text", Qt::red);
    }
    // plot->replot();
    replot();
}

FrmBeamTable::~FrmBeamTable()
{
    delete ui;
}

void FrmBeamTable::setXaxis(QString label, double min, double max)
{
    // Set X axis ranges
    // plot->xAxis->setLabel(label);
    // plot->xAxis->setRange(min, max);
    // plot->xAxis->grid()->setSubGridVisible(true);
    xAxis->setLabel(label);
    xAxis->setRange(min, max);
    xAxis->grid()->setSubGridVisible(true);
}

void FrmBeamTable::setYaxis(QString label, double min, double max)
{
    // Set Y axis ranges
    // plot->yAxis->setLabel(label);
    // plot->yAxis->setRange(min, max);
    // plot->yAxis->grid()->setSubGridVisible(true);
    yAxis->setLabel(label);
    yAxis->setRange(min, max);
    yAxis->grid()->setSubGridVisible(true);
}

void FrmBeamTable::setData(QVector<QVector<double> > data)
{
    mdata = data;
    setGridPoints(mdata);
}

void FrmBeamTable::loadData(QString filename)
{
    QFile readFile(filename);
    if (!readFile.exists()){
        qDebug() << "File not exist: " << filename;
        return;
    }
    if (readFile.open(QIODevice::ReadOnly)) {
        // loadCyntecData(&readFile);
        loadHanwhaData(&readFile);
    }
}

void FrmBeamTable::loadCyntecData(QIODevice *filedevice)
{
    //cyntec data
    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        // std::shared_ptr<QXlsx::Cell> sharedCell;
        // Cell* cellB, cellC, cellD, cellE, cellF;
        QVariant varB, varC, varD;//, varE, varF;
        int irow=0;
        int icol=0;
        auto cell = xlsReader.cellAt(irow, icol);

        //BeamTable
        if (xlsReader.selectSheet("BeamTable")){
            mdata.clear();
            // mBeamTableData->clear();
            irow=2;
            icol=2;
            cell = xlsReader.cellAt(irow, icol);
            if (cell != NULL){
                varB = cell->readValue();
                while (varB.isValid()){
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    irow++;
                    if (irow>95){
                        break;
                    }
                    cell = xlsReader.cellAt(irow, icol);
                    if (cell != NULL){
                        varB = cell->readValue();
                        if(varB.isValid()){
                            cell = xlsReader.cellAt(irow, 3);
                            if (cell != NULL){
                                varC = cell->readValue(); //AZ
                            }
                            cell = xlsReader.cellAt(irow, 4);
                            if (cell != NULL){
                                varD = cell->readValue(); //EL
                            }
                            mdata.append({varB.toDouble(), varC.toDouble(), varD.toDouble()});
                        }else{
                            qDebug() << "BeamTable:No value row:" << irow << " col:" << icol;
                        }
                    }else{
                        // qDebug() << "[TableData]cell is NULL " << irow << "," << icol;
                        break;
                    }
                }
            } else {
                qDebug() << "sheet 'BeamTable' cell of (" << irow << "," << icol << ") is NULL";
            }
        }else{
            qDebug() << "sheet 'BeamTable' not found";
        }
    }

}

void FrmBeamTable::loadHanwhaData(QIODevice *filedevice)
{
    int row=15;
    int endrow=253;

    QVariant varA, varC, varD;//, varE;
    // qDebug() << "xlsReader(filename): " << filename;
    QXlsx::Document xlsReader(filedevice);
    if(xlsReader.load()){
        for(int i=row; i<=endrow;i++){
            QXlsx::Cell* cellA = xlsReader.cellAt(i, 1).get(); // col A
            if ( cellA != NULL )
            {
                varA = cellA->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varA; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 1 fail";
                continue;
            }
            QXlsx::Cell* cellC = xlsReader.cellAt(i, 3).get(); // col C : AZ
            if ( cellC != NULL )
            {
                varC = cellC->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varC; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 3 fail";
                continue;
            }
            QXlsx::Cell* cellD = xlsReader.cellAt(i, 4).get(); // col D : EI
            if ( cellD != NULL )
            {
                varD = cellD->readValue(); // read cell value (number(double), QDateTime, QString ...)
                // qDebug() << varD; // display value. it is 'Hello Qt!'.
            }else{
                qDebug() << "get cell " << i << " x 4 fail";
                continue;
            }
            mdata.append({varA.toDouble(), varC.toDouble(), varD.toDouble()});
        }

    }else{
        qDebug() << "[Hanwha::initBeamData]xlsReader error: ";
    }
}

void FrmBeamTable::setGridPoints(QVector<QVector<double>> data)
{
    QVector<double> az, el;
    QVector<int> beamIds;

    for (const QVector<double> &row : data) {
        if (row.size() >= 3) {
            beamIds.push_back(static_cast<int>(row[0]));
            az.push_back(row[1]);
            el.push_back(row[2]);
        }
    }
    // Create selectable circles and labels for each beam
    for (int i = 0; i < az.size(); ++i) {
        double x = az[i];
        double y = el[i];
        double r = 8;
        // Create a tracer at the desired axis coordinate
        QCPItemTracer *centerTracer = new QCPItemTracer(this);
        centerTracer->setStyle(QCPItemTracer::tsNone);
        centerTracer->position->setType(QCPItemPosition::ptPlotCoords);
        centerTracer->position->setCoords(x, y);

        // Circle for beam point
        QCPItemEllipse *circle = new QCPItemEllipse(this);
        circle->setLayer("items");
        //fix to circle sharp
        // circle->topLeft->setType(QCPItemPosition::ptAbsolute);
        // circle->bottomRight->setType(QCPItemPosition::ptAbsolute);
        // Anchor to center tracer
        circle->topLeft->setParentAnchor(centerTracer->position);
        circle->bottomRight->setParentAnchor(centerTracer->position);
        // Offset in pixels from center
        circle->topLeft->setCoords(-r, -r);
        circle->bottomRight->setCoords(r, r);

        circle->setPen(QPen(Qt::green, 2));
        circle->setBrush(Qt::NoBrush);
        circle->setSelectable(true);
        circle->setSelectedPen(QPen(Qt::red, 3));  // Highlight when selected

        // Store Beam ID for this circle
        circle->setProperty("BeamID", beamIds[i]);
        circle->setProperty("az", QString::number(x));
        circle->setProperty("el", QString::number(y));
        mEllipses.insert(beamIds[i], circle);// Store for later use

        // Add Beam ID label
        QCPItemText *label = new QCPItemText(this);
        // Anchor label to ellipse center
        label->position->setParentAnchor(circle->center);
        // Offset label upward (in plot coordinates)
        label->position->setCoords(0, 3.0); // e.g., offsetY = 1.0
        // Align label so its bottom center touches the anchor point
        label->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
        label->setText(QString::number(beamIds[i]));
        label->setFont(QFont("Arial", 6));
        label->setColor(Qt::black);
        // rssi
        QCPItemText *lb = new QCPItemText(this);
        lb->setLayer("overlay");
        lb->position->setParentAnchor(circle->center);
        // lb->position->setCoords(0, 10.0);
        lb->setPositionAlignment(Qt::AlignCenter | Qt::AlignHCenter);
        lb->setText("");
        lb->setFont(QFont("Arial", 8, QFont::Bold));
        lb->setColor(Qt::black);
        mEllipsesValue.insert(beamIds[i], lb);
    }
    // connect(this, &FrmBeamTable::itemClick)
    // Connect click handler for items
    // QObject::connect(plot, &QCustomPlot::itemClick,
    //                  [&](QCPAbstractItem *item, QMouseEvent *event) {
    //     Q_UNUSED(event)
    //                      if (item->property("BeamID").isValid()) {
    //                          int beamId = item->property("BeamID").toInt();
    //                          qDebug() << "Beam ID clicked:" << beamId;
    //                      }
    //                  });
}

void FrmBeamTable::setEllipseBGColor(int id, QColor bgcolor)
{
    if (mEllipses.contains(id)){
        QCPItemEllipse *ellipse = mEllipses.value(id);
        QBrush brush = QBrush();
        brush.setColor(bgcolor);
        brush.setStyle(Qt::SolidPattern);
        ellipse->setBrush(brush);
    }else{
        qDebug() << "[setEllipseBGColor]Dod not have Ellipse of " << QString::number(id);
    }
}

void FrmBeamTable::clearEllipseBGColor(int id)
{
    if (mEllipses.contains(id)){
        QCPItemEllipse *ellipse = mEllipses.value(id);
        ellipse->setBrush(Qt::NoBrush);
    }else{
        qDebug() << "[clearEllipseBGColor]Dod not have Ellipse of " << QString::number(id);
    }

}

void FrmBeamTable::setEllipse(int id, QString text, QColor bgcolor)
{
    if (mEllipses.contains(id)){
        QCPItemText *lb = mEllipsesValue.value(id);
        lb->setText(text);
        setEllipseBGColor(id, bgcolor);
    }else{
        qDebug() << "[setEllipse]Dod not have Ellipse of " << QString::number(id);
    }
}

void FrmBeamTable::changeEvent(QEvent *e)
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

void FrmBeamTable::mouseMoveEvent(QMouseEvent *event)
{
    QCPAbstractItem* item = itemAt(event->pos(), false);
    if (item && item->selectTest(event->pos(), false) >= 0) {
        if (auto* ellipse = qobject_cast<QCPItemEllipse*>(item)) {
            int idx = mEllipses.key(ellipse);
            QString s ="";
            if (mEllipsesValue.contains(idx)){
               QCPItemText *tx = mEllipsesValue.value(idx);
                s = tx->text();
            }
            QString tip = QString("%1: %2, %3").arg(ellipse->property("BeamID").toString(),
                                                  ellipse->property("az").toString(),
                                               ellipse->property("el").toString());
            if (!s.isEmpty()){
                tip = tip + "\n"+ s;
            }
            QToolTip::showText(event->globalPosition().toPoint(), tip, this);
            return;
        }
    }
    QToolTip::hideText();
}
