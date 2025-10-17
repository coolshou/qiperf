#ifndef FRMADDRECTANGLE_H
#define FRMADDRECTANGLE_H

#include <QDialog>
#include <QColor>
#include <QStyledItemDelegate>
#include <QPainter>

#include "QGVGlobal.h"

class ColorDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        painter->save();

        QColor color = index.data(Qt::UserRole).value<QColor>();
        QRect rect = option.rect.adjusted(4, 4, -4, -4);

        painter->setBrush(color);
        painter->setPen(Qt::black);
        painter->drawRect(rect);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSize(60, 20);  // Customize size if needed
    }
};

namespace Ui {
class FrmAddRectangle;
}

class FrmAddRectangle : public QDialog
{
    Q_OBJECT

public:
    explicit FrmAddRectangle(QWidget *parent = nullptr);
    ~FrmAddRectangle();
    void setPos(double lat, double lon);
    QGV::GeoPos getPos();
    QString getLable();
    void setLabel(QString label);
    QSize getSize();
    void setSize(QPointF size);
    QColor getColor();
    void setColor(QColor color);
    bool getEditMode();
    void setEditMode(bool bEdit);

public slots:
    void onAccepted();
protected:
    void changeEvent(QEvent *e);

private:
    Ui::FrmAddRectangle *ui;
    double mlatitide;
    double mlontitude;
    bool mEditMode;
    QString mtitle;
};

#endif // FRMADDRECTANGLE_H
