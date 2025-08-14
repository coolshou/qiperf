#include "qgvcolortext.h"



QGVColorText::QGVColorText(const QGV::GeoPos &geoPos, const QSizeF &size)
{
    // setAnchor(, { Qt::BottomEdge });
    setGeometry(geoPos, size);
    // QPainter painter(this);
    // QFont font = painter.font(); // current font used by the painter
    // metrics = new QFontMetrics(font);
}

void QGVColorText::setGeometry(const QGV::GeoPos &geoPos, const QSizeF &size)
{
    mGeoPos = geoPos;
    mProjPos = {};
    mSize = size;
    mProjRect = {};
    calculateGeometry();
}

void QGVColorText::setGeometry(const QPointF &projPos, const QSizeF &size)
{
    mGeoPos = {};
    mProjPos = projPos;
    mSize = size;
    mProjRect = {};
    calculateGeometry();
}

void QGVColorText::onProjection(QGVMap *geoMap)
{
    QGVWidgetText::onProjection(geoMap);
    calculateGeometry();
}

QPainterPath QGVColorText::projShape() const
{
    QPainterPath path;
    path.addRect(mProjRect);
    return path;
}

void QGVColorText::projPaint(QPainter *painter)
{
    if (mProjRect.isEmpty()) {
        qDebug() << "QGVColorText::projPaint NO mProjRect";
        return;
    }
    if (getText().isEmpty()){
        qDebug() << "QGVColorText::projPaint NO Text";
        return;
    }

    QRectF paintRect = mProjRect;
    qDebug() << "paintRect:" << paintRect;
    qDebug() << "mProjPos:" <<mProjPos;
    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    // painter->drawImage(paintRect, getImage());
    painter->drawText(mProjPos, getText());
}

void QGVColorText::calculateGeometry()
{
    if (getMap() == nullptr) {
        return;
    }

    if (!mGeoPos.isEmpty()) {
        mProjPos = getMap()->getProjection()->geoToProj(mGeoPos);
    }
    // QPainter painter(this);
    // QFont font = painter.font(); // current font used by the painter
    // QFontMetrics metrics = QFontMetrics(font);
    // QSize textSize = metrics.size(Qt::TextSingleLine, getText());
    QSize textSize = QSize(10, 10);

    const QSizeF baseSize = !mSize.isEmpty() ? mSize : textSize;
    const QPointF baseAnchor = QPointF(baseSize.width() / 2, baseSize.height() / 2);

    mProjRect = QRectF(mProjPos - baseAnchor, baseSize);
    qDebug() << "mGeoPos:" << mGeoPos
             << " mProjPos:" << mProjPos
             << " mProjRect:" << mProjRect;
    // setAnchor(mProjRect.topLeft().toPoint(), { Qt::BottomEdge });
    // resetBoundary();
    // refresh();
}
