#ifndef QIPCONFIG_H
#define QIPCONFIG_H

#include <QtCore>
#include <QObject>
#include <QByteArray>

//#include "../src/iperfwrapper.h"
#include "../src/iperffileworker.h"
#include "tpplot.h"

class QIPConfigData {

public:
    // QJsonArray tpcfg;  // store iperf config pairs
    QString tpcfg;
    QString env; // store environment setting. eq: PC1 info/PC2 info ...
    QString testdate; // store test datetime of folder which include all record data file
    QStringList datafilenames; // all recored data file name
};

class QIPConfig : public QObject {
    Q_OBJECT
public:
    QIPConfig(QString tmppath, QObject *parent=nullptr);
    bool loadFromFile(const QString &filePath);
    bool saveToFile(const QString &filePath) const;
    uint32_t getVersion();
    QByteArray getTPCfg();
    void setTPCfg(QByteArray tpcfg, QString env="", QString testdate="", QStringList datafilenames={});
    void clear();
    bool importIperf3Log(QString filename);
    bool exportToFile(QString filename, TPPlot *tpplot, int tpwidth=0, int tpheigth=0);
    bool detectSystemProxy(QString &hostname, quint16 &port);

public slots:
    void onProgress(int currentlineno);
    void onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                       QString value, QString unit, QString dir,
                       QString pkt_lost, QString pkt_total);
signals:
    void updateDataPath(QString datapath);
    void updateTPCfg(QByteArray tpcfg);
    void onThroughput(QString refrow, QString sInterval, QString datas); // refrow, sInterval, throughput data
    void updateStartDateTime(QDateTime datetime);
    void updateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                            QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate);
    void updateTPAvg(QString midx, QString sInterval, QString idx,
                     QString value, QString unit, QString dir,
                     QString pkt_lost, QString pkt_total);
    void progress(int currentlineno);

private slots:
    void onThroughputData(int idx, QString sInterval,  QString data);
    void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                        QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate);
private:
    static const QByteArray MAGIC_VALUE;
    static const qint32 VERSION;

    QByteArray serialize() const;
    bool deserialize(const QByteArray &data);
    QByteArray filesToStore(QStringList &inputFiles) const;
    bool filesFromStore(QByteArray &inputData, const QString &outputFolder) const;
    bool parserTPCfgLogFiles(QString logpath);

    // QByteArray storefiles(const QStringList &inputFiles) const;
    // Configuration data
    QString m_tmppath;
    QByteArray m_magic;
    uint32_t m_version;
    uint32_t m_loadversion;
    QIPConfigData *m_data; //compress zip/tar ?
//    IperfWrapper *m_ciperfwrapper;
//    IperfWrapper *m_siperfwrapper;
    QList<IperfFileWorker *> m_fileworkers;

};


#endif // QIPCONFIG_H
