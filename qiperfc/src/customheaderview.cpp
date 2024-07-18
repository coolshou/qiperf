#include "customheaderview.h"

#include <QDebug>

CustomHeaderView::CustomHeaderView(Qt::Orientation orientation, QWidget *parent):
    QHeaderView(orientation, parent)
{
    QFont font = QHeaderView::font();
    m_orgsize = font.pointSize();
}

void CustomHeaderView::setColumnSize(int col, int size)
{
    m_adjcols.insert(col, size);
}

int CustomHeaderView::getFontSize()
{
    return m_orgsize;
}

void CustomHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (m_adjcols.contains(logicalIndex)) { // Change this to your desired column index
        QFont font = painter->font();
        font.setPointSize(m_adjcols.value(logicalIndex)); // Set the desired font size
        painter->setFont(font);
    }
    QHeaderView::paintSection(painter, rect, logicalIndex);
}
