#ifndef FORMCONSOLE_H
#define FORMCONSOLE_H

#include <QWidget>
#include <QSettings>
#include <QtCharts/QChartView>
#include <QMenu>
#include "tpchart.h"


namespace Ui {
class FormConsole;
}

class FormConsole : public QWidget
{
    Q_OBJECT

public:
    explicit FormConsole(QSettings *cfg, QWidget *parent = nullptr);
    ~FormConsole();
    void loadcfg();
    void savecfg();

protected:
    void changeEvent(QEvent *e);

private:
    void init_menufile();
    Ui::FormConsole *ui;
    QMenu *m_menufile;
    QSettings *m_cfg;
    TPChart *m_chart;
    QChartView *chartView;
};

#endif // FORMCONSOLE_H
