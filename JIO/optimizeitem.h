#ifndef OPTIMIZEITEM_H
#define OPTIMIZEITEM_H

#include <QDateTime>
#include <QString>

#include "optimizedata.h"

enum OptimizeColumn {
    TESTDATE = 0,
    ID,
    MCS,
    RSSI,
    SNR,
    C_NAME,
    C_ID,
    C_MCS,
    C_RSSI,
    C_SNR,
    UL,
    DL,
    COLUMN_COUNT
};

class OptimizeItem {
public:
    OptimizeItem(const OptimizeData& data, OptimizeItem* parent = nullptr)
        : m_data(data), m_parentItem(parent) {}

    void appendChild(OptimizeItem* child) { m_childItems.append(child); }
    OptimizeItem* child(int row) const { return m_childItems.value(row); }
    int childCount() const { return m_childItems.size(); }
    OptimizeItem* parentItem() const { return m_parentItem; }
    int row() const {
        return m_parentItem ? m_parentItem->m_childItems.indexOf(const_cast<OptimizeItem*>(this)) : 0;
    }

    const OptimizeData& data() const { return m_data; }
    QDateTime getTestDate() { return m_data.testdate();}

private:
    OptimizeData m_data;
    QVector<OptimizeItem*> m_childItems;
    OptimizeItem* m_parentItem;
};

#endif // OPTIMIZEITEM_H
