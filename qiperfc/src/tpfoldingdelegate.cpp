#include "tpfoldingdelegate.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QTreeView>
#include <QApplication>

#include <QDebug>

TPFoldingDelegate::TPFoldingDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

void TPFoldingDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Draw the default item
    QStyledItemDelegate::paint(painter, opt, index);

    // Determine if the item has children
    if (index.model()->hasChildren(index)) {
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

        QStyleOption branchOption;
        branchOption.initFrom(opt.widget);

        if (opt.state & QStyle::State_Open) {
            branchOption.state = QStyle::State_Children | QStyle::State_Open;
        } else {
            branchOption.state = QStyle::State_Children;
        }

        QRect rect = opt.rect;
        int width = style->pixelMetric(QStyle::PM_IndicatorWidth, &branchOption, opt.widget);
        qDebug() << "TPFoldingDelegate width:" << width;
        int height = style->pixelMetric(QStyle::PM_IndicatorHeight, &branchOption, opt.widget);
//        int x = rect.x() + opt.rect.width() / 2 - width / 2;
        int y = rect.y() + opt.rect.height() / 2 - height / 2;
        int x = rect.x() - width * 0.9; // 2;
//        int y = rect.y();

        // Set custom icon
        QIcon customIcon = (branchOption.state & QStyle::State_Open) ? QIcon(":/folding/open") : QIcon(":/folding/close");
        customIcon.paint(painter, QRect(x, y, width, height), Qt::AlignCenter, QIcon::Normal, QIcon::Off);
    }
}
