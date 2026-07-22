#ifndef MYQCPBARS_H
#define MYQCPBARS_H

#include "qcustomplot.h"
//#include <QCPBars>
#include <QObject>
struct PacketBarData {
    double key;
    double value;       // 通常可以存 丟包率 (pktlost / pkttotal * 100) 或總封包數
    quint64 pktlost;
    quint64 pkttotal;
};

class MyQCPBars : public QCPBars
{
    Q_OBJECT
public:
    MyQCPBars(QCPAxis *keyAxis, QCPAxis *valueAxis);
    void addPacketData(double key, quint64 pktlost, quint64 pkttotal);
    bool getPacketData(double key, PacketBarData &outData) const;
    void clearPacketData();
    void setData(QSharedPointer<QCPBarsDataContainer> data);
    void setData(const QVector<double> &keys, const QVector<double> &values, bool alreadySorted=false);
    void setData(const QVector<double> &keys, const QVector<int> &lostvalues,
                 const QVector<int> &totalvalues , bool alreadySorted=false);
    void addData(double key, double value);
    int getValue(double key, double &value);
    void updateValue(double keyToUpdate, double newvalue);
    double sumValue(double keyToUpdate, double newvalue);
    void clear();
    void setDirection(int iDir);
    int getDirection();
signals:
    // void dataAdded(double key, double value);
    void datasSetted(QSharedPointer<QCPBarsDataContainer> data);

private:
    QVector<double> elementWiseDivision(const QVector<int>& vector1, const QVector<int>& vector2);
    int m_dir; // 0: Tx, 1: Rx
    QMap<double, PacketBarData> m_packetDataMap;
};

#endif // MYQCPBARS_H
