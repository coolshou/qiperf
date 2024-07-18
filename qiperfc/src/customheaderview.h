#ifndef CUSTOMHEADERVIEW_H
#define CUSTOMHEADERVIEW_H

#include <Qt>
#include <QHeaderView>
#include <QFont>
#include <QPainter>

class CustomHeaderView : public QHeaderView
{
    Q_OBJECT

public:
    CustomHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    void setColumnSize(int col, int size);
    int getFontSize();
protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;
private:
    QMap<int, int> m_adjcols; // col, size
    int m_orgsize;

};

#endif // CUSTOMHEADERVIEW_H
