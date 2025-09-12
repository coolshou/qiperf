#ifndef QCPITEMTRIANGLE_H
#define QCPITEMTRIANGLE_H

#include <QObject>

#include "qcustomplot.h"

class QCPItemTriangle : public QCPAbstractItem
{
    Q_OBJECT
public:
    explicit QCPItemTriangle(QCustomPlot *parentPlot);
    void setCenter(const QPointF &center);
    void setSize(double size);
    qreal getX();
    qreal getY();
    // reimplemented virtual methods:
    virtual double selectTest(const QPointF &pos, bool onlySelectable, QVariant *details=nullptr) const Q_DECL_OVERRIDE;

    QCPItemPosition * const topLeft;
    QCPItemPosition * const bottomRight;

protected:
    QPointF plotToPixel(const QPointF &plotCoord) const;
    void draw(QCPPainter *painter) override;
    QPointF anchorPixelPosition(int anchorId) const override;
private:
    QVector<QPointF> computeVertices() const;
    QPointF mCenter;
    double mSize;
};

#endif // QCPITEMTRIANGLE_H
