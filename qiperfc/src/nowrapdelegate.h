#ifndef NOWRAPDELEGATE_H
#define NOWRAPDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

class NoWrapDelegate : public QStyledItemDelegate
{
public:
    explicit NoWrapDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // NOWRAPDELEGATE_H
