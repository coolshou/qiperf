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

QSize NoWrapDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);

    // Force the height to a fixed value (e.g., 24 pixels),
    // or base it on the font metrics for a clean single line:
    int fontHeight = option.fontMetrics.height();
    size.setHeight(fontHeight + 6); // Font height + 6px padding (3px top/bottom)

    return size;
}
