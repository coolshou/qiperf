#include "numberdelegate.h"

NumberDelegate::NumberDelegate(NumberType type, QObject *parent)
    : QStyledItemDelegate{parent}, m_numberType(type)
{}

QWidget *NumberDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QLineEdit *editor = new QLineEdit(parent);

    if (m_numberType == Integer) {
        // For integers, set a QIntValidator
        // You can specify min/max values if needed, e.g., new QIntValidator(0, 100, editor)
        editor->setValidator(new QIntValidator(editor));
    } else { // Double
        // For doubles, set a QDoubleValidator
        // You can specify min/max values and decimals, e.g., new QDoubleValidator(0.0, 100.0, 2, editor)
        editor->setValidator(new QDoubleValidator(editor));
    }

    return editor;
}

void NumberDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Get the current data from the model and set it to the editor
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);
}

void NumberDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{// Get the data from the editor and set it back to the model
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);

}
