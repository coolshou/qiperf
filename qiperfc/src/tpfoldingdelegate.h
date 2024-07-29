#ifndef TPFOLDINGDELEGATE_H
#define TPFOLDINGDELEGATE_H

#include <QStyledItemDelegate>
//#include <QObject>

class TPFoldingDelegate : public QStyledItemDelegate
{
//    Q_OBJECT
public:
    explicit TPFoldingDelegate(QObject *parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

signals:

};

#endif // TPFOLDINGDELEGATE_H
