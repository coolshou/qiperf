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
    explicit NumberDelegate(NumberType type, QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
private:
    NumberType m_numberType;
};

#endif // NUMBERDELEGATE_H
