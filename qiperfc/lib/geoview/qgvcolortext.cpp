#include "qgvcolortext.h"



QGVColorText::QGVColorText()
{
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
        return;
    }
    if (getText().isEmpty()){
        return;
    }

    QRectF paintRect = mProjRect;

    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    // painter->drawImage(paintRect, getImage());
    painter->drawText(paintRect, getText());
}

void QGVColorText::calculateGeometry()
{
    if (getMap() == nullptr) {
        return;
    }

    if (!mGeoPos.isEmpty()) {
        mProjPos = getMap()->getProjection()->geoToProj(mGeoPos);
    }
    QPainter painter(this);
    QFont font = painter.font(); // current font used by the painter
    QFontMetrics metrics = QFontMetrics(font);
    QSize textSize = metrics.size(Qt::TextSingleLine, getText());

    const QSizeF baseSize = !mSize.isEmpty() ? mSize : textSize;
    const QPointF baseAnchor = QPointF(baseSize.width() / 2, baseSize.height() / 2);

    mProjRect = QRectF(mProjPos - baseAnchor, baseSize);
    qDebug() << "mGeoPos:" << mGeoPos
             << " mProjPos:" << mProjPos
             << " mProjRect:" << mProjRect;
    // resetBoundary();
    // refresh();
}
