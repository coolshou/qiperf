#ifndef QIPCONFIG_H
#define QIPCONFIG_H

#include <QtCore>
#include <QObject>
#include <QByteArray>
// #include <QJsonArray>

class QIPConfigData {

public:
    // QJsonArray tpcfg;  // store iperf config pairs
    QString tpcfg;
};

class QIPConfig : public QObject {
    Q_OBJECT
public:
    QIPConfig(QObject *parent=nullptr);
    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath) const;
    uint32_t getVersion();
    QByteArray getTPCfg();
    void setTPCfg(QByteArray tpcfg);

private:
    static const QByteArray MAGIC_VALUE;
    static const QByteArray VERSION;

    QByteArray serialize() const;
    bool deserialize(const QByteArray &data);
    // Configuration data
    QByteArray m_magic;
    uint32_t m_version;
    QIPConfigData *m_data; //compress zip/tar ?


};


#endif // QIPCONFIG_H
