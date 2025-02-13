#include "nowrapdelegate.h"

NoWrapDelegate::NoWrapDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void NoWrapDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.textElideMode = Qt::ElideRight; // This will truncate the text with an ellipsis if it doesn't fit
    QStyledItemDelegate::paint(painter, opt, index);
}
