#ifndef FORMQIPERFDS_H
#define FORMQIPERFDS_H

#include <QWidget>
#include <QAbstractItemModel>

namespace Ui {
class FormQIperfds;
}
// GUI to show QIperfd list
class FormQIperfds : public QWidget
{
    Q_OBJECT

public:
    explicit FormQIperfds(QWidget *parent = nullptr);
    ~FormQIperfds();
    void setModel(QAbstractItemModel *model);
    void setColumnWidth(int column, int width);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::FormQIperfds *ui;
};

#endif // FORMQIPERFDS_H
