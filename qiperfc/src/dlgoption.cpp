#include "dlgoption.h"
#include "ui_dlgoption.h"

#include <QFontDatabase>
#include <QStringList>

#include "comm.h"
#include "tpgroup.h"

#include <QDebug>

dlgOption::dlgOption(QSettings *cfg, QWidget *parent) :
//FormOption::FormOption(QSettings *cfg, QStringList interfaces, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgOption)
{
    ui->setupUi(this);
    // hidetab("main");
    // font init
    initFonts();

    m_cfg = cfg;
//    ui->cb_minterfaces->addItems(interfaces);
    loadcfg(cfg);
    connect(ui->sbCpuCheckInterval, &QSpinBox::valueChanged, this , &dlgOption::onCpuCheckIntervalValueChanged);
    connect(ui->sbMemCheckInterval, &QSpinBox::valueChanged, this , &dlgOption::onMemCheckIntervalValueChanged);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &dlgOption::onAccept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &dlgOption::onReject);
    connect(ui->sb_width_tp, QOverload<int>::of(&QSpinBox::valueChanged), this, &dlgOption::onWidthChange);
    connect(ui->sb_heigth_tp, QOverload<int>::of(&QSpinBox::valueChanged), this, &dlgOption::onHeigthChange);


    ui->TPGroup->setId(ui->rbTPGroupAll, static_cast<int>(TPGroup::GroupMode::Total));
    ui->TPGroup->setId(ui->rbTPGroupEach, static_cast<int>(TPGroup::GroupMode::Detail));
    ui->TPGroup->setId(ui->rbTPGroupDirection, static_cast<int>(TPGroup::GroupMode::Direction));
    ui->TPGroup->setId(ui->rbTPGroupComment, static_cast<int>(TPGroup::GroupMode::Comment));
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    connect(ui->TPGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &dlgOption::onTPGroupTypeChange);
#else
    connect(ui->TPGroup, &QButtonGroup::idClicked, this, &dlgOption::onTPGroupTypeChange);
#endif
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
    // connect(ui->cb_TPGroup, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onStateChanged);
    connect(ui->cb_IgnoreWrongInterval, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onIgnoreWrongIntervalChanged);
#else
    // connect(ui->cb_TPGroup, &QCheckBox::checkStateChanged, this, &dlgOption::onStateChanged);
    connect(ui->cb_IgnoreWrongInterval, &QCheckBox::checkStateChanged, this, &dlgOption::onIgnoreWrongIntervalChanged);
#endif

    connect(ui->cb_TPUnit, &QComboBox::currentTextChanged, this, &dlgOption::onTPUnitChanged);
    ui->tabWidget->setCurrentIndex(0);
    connect(ui->cbCpuCheck, &QCheckBox::clicked, this, &dlgOption::onCpuCheckClicked);
    connect(ui->cbMemCheck, &QCheckBox::clicked, this, &dlgOption::onMemCheckClicked);
}

dlgOption::~dlgOption()
{
    delete ui;
}

void dlgOption::loadcfg(QSettings *cfg)
{
    int midx=0;
    //load cfg to ui
    cfg->beginGroup("MainWindow");
    ui->sbCpuCheckInterval->setValue(cfg->value("CpuCheckInterval", 1).toInt());
    if (ui->sbCpuCheckInterval->value()>0){
        ui->cbCpuCheck->setChecked(true);
    }
    ui->sbMemCheckInterval->setValue(cfg->value("MemCheckInterval", 1).toInt());
    if (ui->sbMemCheckInterval->value()>0){
        ui->cbMemCheck->setChecked(true);
    }
    cfg->endGroup();

    cfg->beginGroup("Iperf");
    ui->sb_WaitServerReady->setValue(cfg->value("WaitServerReady", 10).toInt());
    ui->sb_width_tp->setValue(cfg->value("TPExportWidth", 1280).toInt());
    ui->sb_heigth_tp->setValue(cfg->value("TPExportHeigth", 500).toInt());
    // ui->cb_TPGroup->setChecked(cfg->value("TPGroup", false).toBool());
    // ui->gbTPGroup->
    int tpgrouptype = cfg->value("TPGroupType", 0).toInt();
    if (tpgrouptype== static_cast<int>(TPGroup::GroupMode::Detail)) {
        ui->rbTPGroupEach->setChecked(true);
    }else if (tpgrouptype== static_cast<int>(TPGroup::GroupMode::Total)) {
        ui->rbTPGroupAll->setChecked(true);
    }else if (tpgrouptype== static_cast<int>(TPGroup::GroupMode::Direction)) {
        ui->rbTPGroupDirection->setChecked(true);
    }else if (tpgrouptype== static_cast<int>(TPGroup::GroupMode::Comment)) {
        ui->rbTPGroupComment->setChecked(true);
    }else {
        ui->rbTPGroupEach->setChecked(true);
    }

    midx = ui->cb_TPUnit->findText(cfg->value("TPUnit", "Mbits/sec").toString());
    if (midx>=0){
        ui->cb_TPUnit->setCurrentIndex(midx);
    }
    ui->cb_IgnoreWrongInterval->setChecked(cfg->value("IgnoreWrongInterval", true).toBool());
    cfg->endGroup();

    cfg->beginGroup("agent");
    // midx = ui->cb_minterfaces->findText(cfg->value("managerifname", "").toString());
    // if (midx>=0){
    //     ui->cb_minterfaces->setCurrentIndex(midx);
    // }
    // ui->sb_port->setValue(cfg->value("managerport", 45454).toInt());
    ui->cbCloseToSysTray->setChecked(cfg->value("closetosystray", false).toBool());
    cfg->endGroup();

    cfg->beginGroup("notice");
    ui->cb_showManagerIPWarning->setChecked(cfg->value("showManagerIPWarning", true).toBool());
    cfg->endGroup();
    cfg->beginGroup("gps");
    ui->leOpenStreetMapTile->setText(cfg->value("OpenStreetMapTile", "https://tile.openstreetmap.org/{z}/{x}/{y}.png").toString());
    cfg->endGroup();

    cfg->beginGroup("terminal");
    QString fontfamily = cfg->value("FontFamily","Noto Mono").toString();
    int idx = ui->cbFontFamily->findText(fontfamily);
    if (idx){
        ui->cbFontFamily->setCurrentIndex(idx);
    }
    QString fontstyle = cfg->value("FontStyle","Regular").toString();
    idx = ui->cbFontStyle->findText(fontstyle);
    if (idx){
        ui->cbFontStyle->setCurrentIndex(idx);
    }
    ui->sbFontPonitSize->setValue(cfg->value("FontSize", 10).toInt());
    cfg->endGroup();
}

void dlgOption::updatecfg()
{
    //save ui value to cfg
    m_cfg->beginGroup("MainWindow");
    int cpuinterval=0;
    if (ui->cbCpuCheck->isChecked()){
        cpuinterval = ui->sbCpuCheckInterval->value();
    }
    int meminterval=0;
    if (ui->cbMemCheck->isChecked()){
        meminterval = ui->sbMemCheckInterval->value();
    }
    m_cfg->setValue("CpuCheckInterval", cpuinterval);
    m_cfg->setValue("MemCheckInterval", meminterval);
    m_cfg->endGroup();

    m_cfg->beginGroup("Iperf");
    // QString args;
    // m_cfg->setValue("args", args);
    m_cfg->setValue("WaitServerReady", ui->sb_WaitServerReady->value());
    m_cfg->setValue("TPExportWidth", ui->sb_width_tp->value());
    m_cfg->setValue("TPExportHeigth", ui->sb_heigth_tp->value());
    // m_cfg->setValue("TPGroup", ui->cb_TPGroup->isChecked());
    int tpgrouptype = 0;
    if (ui->rbTPGroupAll->isChecked()){
        tpgrouptype = static_cast<int>(TPGroup::GroupMode::Total);
    }
    if (ui->rbTPGroupDirection->isChecked()){
        tpgrouptype = static_cast<int>(TPGroup::GroupMode::Direction);
    }
    if (ui->rbTPGroupComment->isChecked()){
        tpgrouptype = static_cast<int>(TPGroup::GroupMode::Comment);
    }
    m_cfg->setValue("TPGroupType", tpgrouptype);

    m_cfg->setValue("TPUnit", ui->cb_TPUnit->currentText());
    m_cfg->setValue("IgnoreWrongInterval", ui->cb_IgnoreWrongInterval->isChecked());
    m_cfg->endGroup();

    m_cfg->beginGroup("agent");
    // QString ifname = ui->cb_minterfaces->currentText();
    // m_cfg->setValue("managerifname", ifname);
    // QStringList qs = ifname.split(": ");
    // int port = ui->sb_port->value();
    // if (qs.length()>=2){
    //     emit ipaddressUpdated(qs[1], port);
    // }
    // m_cfg->setValue("managerport", port);
    m_cfg->setValue("closetosystray", ui->cbCloseToSysTray->isChecked());
    m_cfg->endGroup();

    m_cfg->beginGroup("notice");
    m_cfg->setValue("showManagerIPWarning", ui->cb_showManagerIPWarning->isChecked());
    m_cfg->endGroup();

    m_cfg->beginGroup("gps");
    m_cfg->setValue("OpenStreetMapTile", ui->leOpenStreetMapTile->text());
    emit updateOpenStreetMapTile(ui->leOpenStreetMapTile->text());
    m_cfg->endGroup();

    m_cfg->beginGroup("terminal");
    m_cfg->value("FontFamily",ui->cbFontFamily->currentText());
    m_cfg->value("FontStyle",ui->cbFontStyle->currentText());
    m_cfg->value("FontSize",ui->sbFontPonitSize->value());
    m_cfg->endGroup();

    m_cfg->sync();
}

void dlgOption::setWaitServerReady(int val)
{
    if ((val >= ui->sb_WaitServerReady->minimum()) &&
        (val <= ui->sb_WaitServerReady->maximum())){
        ui->sb_WaitServerReady->setValue(val);
    }
}

int dlgOption::getWaitServerReady()
{
    return ui->sb_WaitServerReady->value();
}

bool dlgOption::getShowManagerIPWarning()
{
    return ui->cb_showManagerIPWarning->isChecked();
}

QString dlgOption::getFontName()
{
    return ui->cbFontFamily->currentText();
}

QString dlgOption::getFontStyle()
{
    return ui->cbFontStyle->currentText();
}

int dlgOption::getFontSize()
{
    return ui->sbFontPonitSize->value();
}

QStringList dlgOption::getSysFontFamilies()
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QFontDatabase db;
    QStringList families = db.families();
    return families;
#else
    return QFontDatabase::families();
#endif

}

QStringList dlgOption::getFontStyles(QString fontfamily)
{
    if (fontfamily.isEmpty()){
        return QStringList();
    }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QFontDatabase db;
    return db.styles(fontfamily);
#else
    return QFontDatabase::styles(fontfamily);
#endif
}

// void dlgOption::setShowGroup(bool bShow)
// {
// #if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
//     disconnect(ui->cb_TPGroup, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onStateChanged);
// #else
//     disconnect(ui->cb_TPGroup, &QCheckBox::checkStateChanged, this, &dlgOption::onStateChanged);
// #endif
// // #endif
//     ui->cb_TPGroup->setChecked(bShow);
// #if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
//     connect(ui->cb_TPGroup, QOverload<int>::of(&QCheckBox::stateChanged), this, &dlgOption::onStateChanged);
// #else
//     connect(ui->cb_TPGroup, &QCheckBox::checkStateChanged, this, &dlgOption::onStateChanged);
// #endif
// }

void dlgOption::onSetTPGroupType(int grouptype)
{
    if (grouptype == static_cast<int>(TPGroup::GroupMode::Total)){
        ui->rbTPGroupAll->setChecked(true);
    }
    if (grouptype == static_cast<int>(TPGroup::GroupMode::Detail)){
        ui->rbTPGroupEach->setChecked(true);
    }
    if (grouptype == static_cast<int>(TPGroup::GroupMode::Direction)){
        ui->rbTPGroupDirection->setChecked(true);
    }
    if (grouptype == static_cast<int>(TPGroup::GroupMode::Comment)){
        ui->rbTPGroupComment->setChecked(true);
    }
}

void dlgOption::onTPUnitChanged(QString sunit)
{
    emit updateTPUnit(sunit);
}

void dlgOption::updateFontStyle(QString fontfamily)
{
    QStringList ffs = getFontStyles(fontfamily);
    if (ffs.length()>0){
        ui->cbFontStyle->clear();
        ui->cbFontStyle->addItem("");
        ui->cbFontStyle->addItems(ffs);
    }
}

void dlgOption::onCpuCheckClicked(bool checked)
{
    int value=0;
    ui->sbCpuCheckInterval->setEnabled(checked);
    if (checked){
        if(ui->sbCpuCheckInterval->value()==0){
            value=1;
            ui->sbCpuCheckInterval->setValue(value);
        }
    }
    emit updateCpuCheckInterval(value);
}

void dlgOption::onMemCheckClicked(bool checked)
{
    int value=0;
    ui->sbMemCheckInterval->setEnabled(checked);
    if (checked){
        if(ui->sbMemCheckInterval->value()==0){
            value=1;
            ui->sbMemCheckInterval->setValue(value);
        }
    }
    emit updateMemCheckInterval(value);
}

void dlgOption::onCpuCheckIntervalValueChanged(int value)
{
    emit updateCpuCheckInterval(value);
    if (value==0){
        ui->cbCpuCheck->setChecked(false);
        ui->sbCpuCheckInterval->setEnabled(false);
    }else{
        ui->cbCpuCheck->setChecked(true);
        ui->sbCpuCheckInterval->setEnabled(true);
    }
}

void dlgOption::onMemCheckIntervalValueChanged(int value)
{
    emit updateMemCheckInterval(value);
    if (value==0){
        ui->cbMemCheck->setChecked(false);
        ui->sbMemCheckInterval->setEnabled(false);
    }else{
        ui->cbMemCheck->setChecked(true);
        ui->sbMemCheckInterval->setEnabled(true);
    }
}

// void dlgOption::setTPsize(int width, int heigth)
// {
//     // ui->sb_width_tp->setValue(width);
//     // ui->sb_heigth_tp->setValue(heigth);
// }

void dlgOption::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void dlgOption::hidetab(QString tabname)
{
    int i;
    for (i = 0; i < ui->tabWidget->count(); ++i) {
        if (ui->tabWidget->tabText(i) == tabname) {
            // return parent;
            break;
        }
    }
    // qDebug() << "hidetab : " << tabname << " idx:" << QString::number(i);
    ui->tabWidget->removeTab(i);
}

void dlgOption::initFonts()
{
    QStringList fs = getSysFontFamilies();
    ui->cbFontFamily->addItem("");
    ui->cbFontFamily->addItems(fs);
    connect(ui->cbFontFamily, &QComboBox::currentTextChanged, this , &dlgOption::updateFontStyle);
}

void dlgOption::onReject()
{
    this->close();
}


void dlgOption::onAccept()
{
    updatecfg();
    this->close();
}

void dlgOption::onWidthChange(int width)
{
    emit widthChanged(width);
}

void dlgOption::onHeigthChange(int heigth)
{
    emit heigthChanged(heigth);
}

void dlgOption::onStateChanged(int state)
{
    if (state == Qt::Checked){
        emit showGroup(true);
        m_cfg->setValue("Iperf/TPGroup", true);
    }else{
        emit showGroup(false);
        m_cfg->setValue("Iperf/TPGroup", false);
    }
}
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
void dlgOption::onTPGroupTypeChange(QAbstractButton *button)
#else
void dlgOption::onTPGroupTypeChange(int id)
#endif
{
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    int id = 0;
    qDebug() << "onTPGroupTypeChange: " << button;
#else
    qDebug() << "onTPGroupTypeChange: "  << QString::number(id);
#endif
    emit setTPGroupType(id);
    m_cfg->setValue("Iperf/TPGroupType", id);
}

void dlgOption::onIgnoreWrongIntervalChanged(int state)
{
    if (state == Qt::Checked){
        emit IgnoreWrongInterval(true);
        m_cfg->setValue("Iperf/IgnoreWrongInterval", true);
    }else{
        emit IgnoreWrongInterval(false);
        m_cfg->setValue("Iperf/IgnoreWrongInterval", false);
    }
}

