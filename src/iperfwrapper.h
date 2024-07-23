#ifndef IPERFWRAPPER_H
#define IPERFWRAPPER_H

#include <QObject>
#include <QMap>
#include <QJsonArray>

class IperfWrapper : public QObject
{
    Q_OBJECT
public:
    explicit IperfWrapper(QObject *parent = nullptr);

    QString toIperf3args(QVariantMap jsondata); // to iperf3 args
    QString toIperf2args(QVariantMap jsondata); // to iperf2 args
    void parserIperf3(QString msg);
    void setSetting(int idx, bool servermode, QString parallel, bool bidir, QString bidirtag);
    void setFile(QString filename);
    void setIperf(QString version, QString protocal);
public slots:
    void work();
signals:
    void sendThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data

private:

private:
    int m_idx;  // ref row
    bool m_servermode=false;
    QString m_parallel="0";
    bool m_bidir=false;
    QString m_bidirtag;
    QString m_filename;
    QMap<QString, QJsonArray> m_tpdatas;
    QString m_version;
    QString m_protocal;
};

#endif // IPERFWRAPPER_H
