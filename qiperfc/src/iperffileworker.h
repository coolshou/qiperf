#ifndef IPERFFILEWORKER_H
#define IPERFFILEWORKER_H

#include <QObject>
#include <QThread>
#include <QVector>

#include "iperfwrapper.h"
class TPData: public QObject
{
public:
    QVector<double> timeDatas;
    QVector<double> valueDatas;
    QVector<int> packetLost;
    QVector<int> packetTotal;
    QVector<double> lostrate;
};

class IperfFileWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfFileWorker(QString version, QString protocal,
                             int idx, bool servermode, int parallel,
                             bool bidir, QString bidirtag , QString filename,
                             int delay=0, uint interval=1,
                             QObject *parent = nullptr);
    ~IperfFileWorker();
    void start();
public slots:
    void onProgress(QString filename, int currentlineno);

signals:
    void onThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data
    void updateTPDatas(QString idx, QVector<double> timedatas, QVector<double> valuedatas,
                        QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate);
    void updateTPAvg(QString midx, QString sInterval, QString idx,
                     QString value, QString unit, QString dir,
                     QString pkt_lost, QString pkt_total);
    void progress(QString filename, int currentlineno);
private slots:
    void onThroughputData(int midx, QString sInterval,  QString data);
    void onWorkFinished();
private:
    QThread *m_thread;
    IperfWrapper *m_iperfwrapper;
//    QMap<
    QString m_version;
    QString m_protocal;
    int m_idx;
    bool m_servermode;
    int m_parallel;
    bool m_bidir;
    QString m_bidirtag;
    QString m_filename;
    int m_delay;
    TPData *tpdata;
    QMap<QString, TPData*> m_datas;
};

#endif // IPERFFILEWORKER_H
