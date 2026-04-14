#ifndef IPERFWRAPPER_H
#define IPERFWRAPPER_H
/*
 * this class will be run in QThread
*/
#include <QObject>
#include <QMap>
#include <QJsonArray>
#include <QString>
#include <QElapsedTimer>

class IperfWrapper : public QObject
{
    Q_OBJECT
public:
    explicit IperfWrapper(bool ignorewronginterval = false, QObject *parent = nullptr);

    QString toIperf3args(QVariantMap jsondata); // to iperf3 args
    QString toIperf2args(QVariantMap jsondata); // to iperf2 args
    void parserIperf2(QString linedata); // parser Iperf2 output line log
    void parserIperf3(QString linedata); // parser Iperf3 output line log
    QString getIdx(QString linedata, QString &idx);
    void setSetting(int refrow, bool servermode, QString parallel, bool bidir, QString bidirtag);
    void setFile(QString filename);
    void setIperf(QString version, QString protocal, uint port);
    void setDuration(uint duration);
    void setDelaytime(int delaytime);
    void setRestarttimeoffset(qint64 restarttimeoffset); //
    void setInterval(uint interval);
    void setArgs(QString arg);
    void setOmit(int omit);
    void debug(QString msg, int debuglv=1);
    void setDebugLevel(int lv);

public slots:
    void work();
signals:
    void sendThroughput(int idx, QString sInterval,  QString data); // refrow, sInterval, throughput data
    void workFinished();
    void progress(QString filename, int currentlineno);
    void debuginfo(QString msg);
    void iperf2ended();// indicate iperf2 server should stop
private:
    // int getTimeStempLength(QString timestempformat);
    qint64 getTimeStempLength(const std::string& format_string);
    long long countLines(const QString &fileName);
    long long countLinesFast(const QString &fileName);

private:
    int m_refrow;  // ref row
    bool m_servermode=false;
    QString m_parallel="0";
    bool m_bidir=false;
    QString m_bidirtag;
    QString m_filename;
    QMap<QString, QJsonArray> m_tpdatas;
    QMap<QString, QString> m_idxdir; // record each idx's direction (iperf2 use)
    QString m_version;
    QString m_protocal;
    uint m_port; //use port number;
    int m_delaytime;
    qint64 m_restarttimeoffset; // store restart time and init start time diff in sec
    uint m_interval;
    uint mDuration; // -t
    int m_omit=0;
    bool m_ignorewronginterval;
    bool bWithtimestamp=false;
    qint64 iTimestampLength=0;
    QStringList m_arguments;  //iperf args
    int m_debuglv;
    int endcount;
    QElapsedTimer updatetimer;
};

#endif // IPERFWRAPPER_H
