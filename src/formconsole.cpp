#include "formconsole.h"
#include "ui_formconsole.h"

#include <QSplitter>
#include <QScreen>
#include <QAction>
#include <QIcon>

QT_CHARTS_USE_NAMESPACE

FormConsole::FormConsole(QSettings *cfg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormConsole)
{
    ui->setupUi(this);
    m_cfg = cfg;
    init_menufile();
    loadcfg();

    QSplitter *sp = new QSplitter(Qt::Vertical, this);
    m_chart = new TPChart();
//    m_chart->setTitle("Throughput");
//    m_chart->setAnimationOptions(QChart::AllAnimations);
    chartView= new QChartView(m_chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    sp->setStyleSheet("QSplitter::handle{background-color:gray}");
//    ui->verticalLayout->addWidget(sp);
    sp->addWidget(ui->treeWidget);
    sp->addWidget(ui->w_chart);
    ui->verticalLayout->addWidget(sp);
    ui->vl_chart->addWidget(chartView);
    m_chart->show();
//    chartView.show();


}

FormConsole::~FormConsole()
{
    savecfg();
    delete ui;
}

void FormConsole::loadcfg()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int l = (screenGeometry.width() - 1024)/2;
    int t = (screenGeometry.height() - 768)/2;
    //load config
    m_cfg->beginGroup("main");
    QRect rect = m_cfg->value("geometry", QRect(l,t, 1024, 768)).toRect();
    setGeometry(rect);
    m_cfg->endGroup();
}

void FormConsole::savecfg()
{
    m_cfg->beginGroup("main");
    m_cfg->setValue("geometry", this->geometry());
    m_cfg->endGroup();
}

void FormConsole::changeEvent(QEvent *e)
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

void FormConsole::init_menufile()
{
    m_menufile = new QMenu();
    QAction *actLoad = new QAction(QIcon(":/load"), "Load");
    QAction *actSave = new QAction(QIcon(":/save"),"Save");
    m_menufile->addAction(actLoad);
    m_menufile->addAction(actSave);
    ui->pb_file->setMenu(m_menufile);

}
