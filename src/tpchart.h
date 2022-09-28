#ifndef TPCHART_H
#define TPCHART_H

#include <QtCharts/QChart>
#include <QObject>
#include <QTimer>
#include <QMap>
#include <QColor>

QT_CHARTS_BEGIN_NAMESPACE
    class QSplineSeries;
    class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class TPChart : public QChart
{
    Q_OBJECT
public:
    TPChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = {});
    virtual ~TPChart();

    void add_series(QString idx, QColor pencolor);
    void update_series_data(QString idx, qreal x, qreal y);
public slots:
    void handleTimeout();

private:
    QTimer m_timer;
    QMap<QString, QSplineSeries*> m_dict_series;

    QSplineSeries *m_series;
    QStringList m_titles;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    qreal m_step;
    qreal m_x;
    qreal m_y;
};

#endif // TPCHART_H
