#include "tplegenditem.h"

TPLegendItem::TPLegendItem(QString text, QPen pen, QCPLegend *parent)
    :QCPAbstractLegendItem(parent), mText(text), mPen(pen)
{

}

void TPLegendItem::draw(QCPPainter *painter)
{
    painter->setFont(font());
    painter->setPen(mPen);
    painter->drawText(rect(), Qt::AlignLeft, mText);
}

QSize TPLegendItem::minimumOuterSizeHint() const
{
    //TODO: depend on text length?
    return QSize(150, 20); // Set a fixed size for the custom legend item
}
