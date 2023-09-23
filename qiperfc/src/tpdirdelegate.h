#ifndef TPDIRDELEGATE_H
#define TPDIRDELEGATE_H

#include <QStyledItemDelegate>
#include <QStandardItem>

class TPDirDelegate : public QStyledItemDelegate
{
public:
    explicit TPDirDelegate(QObject *parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

};

#endif // TPDIRDELEGATE_H
