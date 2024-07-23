#ifndef IPERFFILEWORKER_H
#define IPERFFILEWORKER_H

#include <QObject>
#include <QThread>

#include "iperfwrapper.h"

class IperfFileWorker : public QObject
{
    Q_OBJECT
public:
    explicit IperfFileWorker(QString version, QString protocal,
                             int idx, bool servermode, int parallel,
                             bool bidir, QString bidirtag , QString filename,
                             QObject *parent = nullptr);
    void start();
signals:
    void onThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data
private slots:
    void onThroughputData(int idx, QString sInterval,  QString data);
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
};

#endif // IPERFFILEWORKER_H
