#ifndef TPPLOT_H
#define TPPLOT_H


#include <QObject>
#include <QTimer>
//#include <QCustomPlot>
#include "lib/qcustomplot.h"
#include "comm.h"
//#include <QPrivateSignal>



class TPPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit TPPlot(QWidget *parent = nullptr);

private slots:
    void realtimeDataSlot(QPrivateSignal sig);
private:
    void initCustomPlote();
    void addRandomGraph();
    QPen newColorPen(int r, int g, int b, int width);
    QTimer dataTimer;

};

#endif // TPPLOT_H
