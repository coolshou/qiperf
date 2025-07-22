#ifndef NUMBERDELEGATE_H
#define NUMBERDELEGATE_H

#include <QStyledItemDelegate>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QLineEdit> // Required for the editor

class NumberDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum NumberType {
        Integer,
        Double
    };
    explicit NumberDelegate(NumberType type,
                            double bottom, double top, int decimals,
                            QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void setRange(double bottom, double top, int decimals);
private:
    NumberType m_numberType;
    double m_bottom;
    double m_top;
    int m_decimals;
};

#endif // NUMBERDELEGATE_H
