#include "qcpitemtriangle.h"

QCPItemTriangle::QCPItemTriangle(QCustomPlot *parentPlot)
    :QCPAbstractItem(parentPlot), topLeft(createPosition("topLeft")),
    bottomRight(createPosition("bottomRight"))
{
    mCenter=QPointF(0,0);
    mSize = 1;
    setSelectable(true);
}

void QCPItemTriangle::setCenter(const QPointF &center)
{
    mCenter = center;
}

void QCPItemTriangle::setSize(double size)
{
    mSize = size;
}

qreal QCPItemTriangle::getX()
{
    return mCenter.x();
}

qreal QCPItemTriangle::getY()
{
    return mCenter.y();
}

double QCPItemTriangle::selectTest(const QPointF &pos, bool onlySelectable, QVariant *details) const
{
    Q_UNUSED(details)
    if (onlySelectable && !selectable()){
        return -1;
    }
    QVector<QPointF> vertices = computeVertices(); // Your method to get the 3 triangle points
    if (vertices.size() != 3)
        return -1;

    // Use barycentric coordinates to test if point is inside triangle
    const QPointF &A = vertices[0];
    const QPointF &B = vertices[1];
    const QPointF &C = vertices[2];

    double denom = (B.y() - C.y()) * (A.x() - C.x()) + (C.x() - B.x()) * (A.y() - C.y());
    double alpha = ((B.y() - C.y()) * (pos.x() - C.x()) + (C.x() - B.x()) * (pos.y() - C.y())) / denom;
    double beta  = ((C.y() - A.y()) * (pos.x() - C.x()) + (A.x() - C.x()) * (pos.y() - C.y())) / denom;
    double gamma = 1.0 - alpha - beta;

    if (alpha >= 0 && beta >= 0 && gamma >= 0) {
        // Point is inside triangle
        if (details)
            *details = QString("Inside triangle");
        return 0.0; // Strong selection
    }

    return -1; // Not selected
}

QPointF QCPItemTriangle::plotToPixel(const QPointF &plotCoord) const
{
    return QPointF(mParentPlot->xAxis->coordToPixel(plotCoord.x()),
                   mParentPlot->yAxis->coordToPixel(plotCoord.y()));
}

void QCPItemTriangle::draw(QCPPainter *painter) {
    QVector<QPointF> vertices = computeVertices();

    QPolygonF triangle;
    for (const QPointF &pt : vertices){
        triangle << plotToPixel(pt);
    }

    qDebug() << "Drawing triangle at center:" << mCenter;

    painter->setPen(QPen(Qt::red, 2));
    painter->setBrush(QBrush(Qt::red));
    painter->drawPolygon(triangle);
}

QPointF QCPItemTriangle::anchorPixelPosition(int anchorId) const {
    Q_UNUSED(anchorId)
    return mCenter;
}

QVector<QPointF> QCPItemTriangle::computeVertices() const {
    // Equilateral triangle pointing upward
    double h = mSize * std::sqrt(3) / 2.0;
    return {
        QPointF(mCenter.x(), mCenter.y() + h / 2),               // Top
        QPointF(mCenter.x() - mSize / 2, mCenter.y() - h / 2),   // Bottom left
        QPointF(mCenter.x() + mSize / 2, mCenter.y() - h / 2)    // Bottom right
    };
}
