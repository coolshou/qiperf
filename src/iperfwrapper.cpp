#include "iperfwrapper.h"

#include <QVariantMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

IperfWrapper::IperfWrapper(QObject *parent)
    : QObject{parent}
{

}
QString IperfWrapper::toIperf3args(QVariantMap jsondata)
{
    QString args;
    bool bServer = jsondata["server"].toBool();
    if (bServer){
        args = args + " -s ";
    }
    uint port = jsondata["port"].toUInt();
    if (port>0){
        args = args + " -p " + QString::number(port);
    }
    if (!bServer){
        QString target = jsondata["target"].toString();
        if (!target.isEmpty()){
            args = args + " -c " + target;
        }
    }
    QString bindaddr = jsondata["bind"].toString();
    if (!bindaddr.isEmpty()){
        args = args + " --bind " + bindaddr;
    }
    if (!bServer){
        bool bidir = jsondata["bidir"].toBool();
        if (bidir){
            args = args + " --bidir";
        }
        bool reverse = jsondata["reverse"].toBool();
        if (reverse){
            args = args + " -R";
        }
        uint duration = jsondata["duration"].toUInt();
        if (duration>0){
            args = args + " -t " + QString::number(duration);
        }
    }
    uint interval = jsondata["interval"].toUInt();
    if (interval>0){
        args = args + " -i " + QString::number(interval);
    }
    if (!bServer){
        uint omit = jsondata["omit"].toUInt();
        if (omit>0){
            args = args + " -O " + QString::number(omit);
        }
        uint parallel = jsondata["parallel"].toUInt();
        if (parallel>1){
            args = args + " -P " + QString::number(parallel);
        }
        QString protocal = jsondata["protocal"].toString();
        if (protocal.contains("UDP")){
            args = args + " -u ";
        }
        uint windowsize = jsondata["windowsize"].toUInt();
        if (windowsize>0){
            QString unit_windowsize = jsondata["unit_windowsize"].toString();
            args = args + " -w " +QString::number(windowsize)+ unit_windowsize;
        }
        uint bitrate = jsondata["bitrate"].toUInt();
        if (bitrate>0){
            QString unit_bitrate = jsondata["unit_bitrate"].toString();
            args = args + " -b " +QString::number(bitrate)+ unit_bitrate;
        }
        uint buffer = jsondata["buffer"].toUInt();
        if (buffer>0){
            QString unit_buffer = jsondata["unit_buffer"].toString();
            args = args + " -l " +QString::number(buffer)+ unit_buffer;
        }
        int dscp = jsondata["dscp"].toUInt();
        if ((dscp>=0)&&(dscp<=64)){
            args = args + " --dscp " +QString::number(dscp);
        }
        uint mss = jsondata["mss"].toUInt();
        if (mss>0){
            args = args + " -M " +QString::number(mss);
        }
        int tos = jsondata["tos"].toUInt();
        if ((tos>=0)&&(tos<=255)){
            args = args + " --tos " +QString::number(tos);
        }
    }
    QString fmtreport = jsondata["fmtreport"].toString();
    if (!fmtreport.isEmpty()){
        args = args + " -f " + fmtreport;
    }
    return args;
}

QString IperfWrapper::toIperf2args(QVariantMap jsondata)
{
    Q_UNUSED(jsondata)
    qDebug() << "//TODO: IperfWrapper::toIperf2args";
    return "";
}

void IperfWrapper::parserIperf3(QString linedata)
{
    if (linedata.contains("Server listening")||
        linedata.contains("Accepted connection")||
        linedata.contains("Connecting")||
        linedata.contains("local") ||
        linedata.contains("Interval") ||
        linedata.contains("(omitted)")||
        linedata.contains("- - ")||
        linedata.contains("--")||
        linedata.contains("Reverse mode")){
        //ignore this lines
    }else if(linedata.contains("iperf Done.")){
        // TODO: when see this : iperf run finish!!
    }else if(linedata.contains("sender")||
             linedata.contains("receiver")){
        //TODO: final sumary

    }else{
//            qDebug() << "parserIperf3: " << msg;
        QString sDir = nullptr;
        int iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
        QString idx = linedata.mid(1,3).trimmed();  // extract [ idx]
        linedata = linedata.right(linedata.length()-iS-1);
        QString sTag="";
        if (m_servermode){
            sTag="s";
        }else{
            sTag="c";
        }
        int iparallel = m_parallel.toInt();
        if (m_bidir){
//                iparallel = m_parallel.toInt()*2; // bidir ;
            // in bidir only get
            iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
            sDir = linedata.mid(1,iS-1).trimmed();// server:[TX-S][RX-S], client:[TX-C][RX-C]
            linedata = linedata.right(linedata.length()-iS-1);
            if (sDir.contains("RX")){
                if (m_servermode){
                    sDir = "Tx";
                }else{
                    sDir = "Rx";
                }
                linedata = linedata.trimmed();
//                    qDebug() << "bidir msg:" << msg;
            }else {
                //ignore Tx part data
                return;
            }
        }else{
            sDir = m_bidirtag;
        }
        QStringList data = linedata.split(" ", Qt::SkipEmptyParts);
        QString sInterval  = data[0]; // Interval
//            qDebug() << "idx:" << idx << " ,Interval:" << sInterval
//                     << " ,Bitrate:" << irec->m_value  << " ,unit:" << irec->m_unit;
        if (!m_tpdatas.contains(sInterval)){
//                qDebug() << "new data: " << sInterval;
            QJsonArray lst =QJsonArray();
            m_tpdatas.insert(sInterval, lst);
        }
        if (m_tpdatas[sInterval].count()<iparallel){
            QJsonObject irec = QJsonObject();
            irec.insert("idx", idx+sTag);  // parallel num
            irec.insert("value", data[4]);  // Bitrate
            irec.insert("unit", data[5]);  // Bitrate unit
            if (!sDir.isNull()){
                irec.insert("dir", sDir);  // direction
            }
            if (idx.contains("SUM", Qt::CaseInsensitive)){
                qDebug() << "==linedata==  " << linedata;
            }else{
//                    qDebug() << "m_parallel: " << iparallel << "m_tpdatas length: " << m_tpdatas[sInterval].count();
                m_tpdatas[sInterval].append(irec);
            }
        }
        if ((m_tpdatas[sInterval].count()>=iparallel)&&
             !idx.contains("SUM", Qt::CaseInsensitive)){
            QJsonArray arr = m_tpdatas[sInterval];
            QJsonDocument doc;
            doc.setArray(arr);
//                qDebug() << "m_tpdatas: " << doc.toJson(QJsonDocument::Compact);
            if (sInterval.contains("-")){
                sInterval = sInterval.right(sInterval.indexOf("-"));
            }
            emit sendThroughput(m_idx, sInterval, doc.toJson(QJsonDocument::Compact));

        }

    }
}

void IperfWrapper::setSetting(int idx, bool servermode, QString parallel, bool bidir, QString bidirtag)
{
    m_idx = idx;
    m_servermode = servermode;
    m_parallel = parallel;
    m_bidir = bidir;
    m_bidirtag = bidirtag;
}

/*iperf3 output format
 * TCP
 *
 * Connecting to host 192.168.111.125, port 5201
[  7] local 192.168.111.23 port 45765 connected to 192.168.111.125 port 5201
[ ID] Interval           Transfer     Bitrate         Retr  Cwnd
[  7]   0.00-1.00   sec  2.55 MBytes  21.4 Mbits/sec    0   5.08 KBytes       (omitted)
[  7]   1.00-2.00   sec  2.63 MBytes  22.1 Mbits/sec    0   5.08 KBytes       (omitted)
[  7]   0.00-1.00   sec  2.67 MBytes  22.4 Mbits/sec    0   5.08 KBytes
[  7]   1.00-2.00   sec  2.89 MBytes  24.2 Mbits/sec    0   5.08 KBytes
[  7]   2.00-3.00   sec  2.66 MBytes  22.3 Mbits/sec    0   5.08 KBytes
[  7]   3.00-4.00   sec  2.64 MBytes  22.2 Mbits/sec    0   5.08 KBytes
[  7]   4.00-5.00   sec  2.66 MBytes  22.3 Mbits/sec    0   5.08 KBytes
[  7]   5.00-6.00   sec  2.64 MBytes  22.2 Mbits/sec    0   5.08 KBytes
[  7]   6.00-7.00   sec  2.68 MBytes  22.5 Mbits/sec    0   5.08 KBytes
[  7]   7.00-8.00   sec  2.68 MBytes  22.5 Mbits/sec    0   5.08 KBytes
[  7]   8.00-9.00   sec  2.63 MBytes  22.0 Mbits/sec    0   5.08 KBytes
[  7]   9.00-10.00  sec  2.67 MBytes  22.4 Mbits/sec    0   5.08 KBytes
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Retr
[  7]   0.00-10.00  sec  26.8 MBytes  22.5 Mbits/sec    0             sender
[  7]   0.00-10.00  sec  26.8 MBytes  22.5 Mbits/sec                  receiver
*/
/*

 *
*/
/* UDP
-----------------------------------------------------------
Server listening on 5201 (test #1)
-----------------------------------------------------------
Accepted connection from 192.168.111.23, port 37809
[  7] local 192.168.111.125 port 5201 connected to 192.168.111.23 port 41708
[ ID] Interval           Transfer     Bitrate         Jitter    Lost/Total Datagrams
[  7]   0.00-1.00   sec   129 KBytes  1.05 Mbits/sec  0.009 ms  0/91 (0%)  (omitted)
[  7]   1.00-2.00   sec   127 KBytes  1.04 Mbits/sec  0.007 ms  0/90 (0%)  (omitted)
[  7]   0.00-1.00   sec   129 KBytes  1.05 Mbits/sec  0.011 ms  0/91 (0%)
[  7]   1.00-2.00   sec   127 KBytes  1.04 Mbits/sec  0.008 ms  0/90 (0%)
[  7]   2.00-3.00   sec   129 KBytes  1.05 Mbits/sec  0.013 ms  0/91 (0%)
[  7]   3.00-4.00   sec   129 KBytes  1.05 Mbits/sec  0.013 ms  0/91 (0%)
[  7]   4.00-5.00   sec   127 KBytes  1.04 Mbits/sec  0.012 ms  0/90 (0%)
[  7]   5.00-6.00   sec   129 KBytes  1.05 Mbits/sec  0.009 ms  0/91 (0%)
[  7]   6.00-7.00   sec   127 KBytes  1.04 Mbits/sec  0.009 ms  0/90 (0%)
[  7]   7.00-8.00   sec   129 KBytes  1.05 Mbits/sec  0.024 ms  0/91 (0%)
[  7]   8.00-9.00   sec   127 KBytes  1.04 Mbits/sec  0.011 ms  0/90 (0%)
[  7]   9.00-10.00  sec   129 KBytes  1.05 Mbits/sec  0.013 ms  0/91 (0%)
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Jitter    Lost/Total Datagrams
[  7]   0.00-10.00  sec  1.25 MBytes  1.05 Mbits/sec  0.013 ms  0/906 (0%)  receiver

Connecting to host 192.168.111.125, port 5201
[  7] local 192.168.111.23 port 41708 connected to 192.168.111.125 port 5201
[ ID] Interval           Transfer     Bitrate         Total Datagrams
[  7]   0.00-1.00   sec   129 KBytes  1.05 Mbits/sec  91  (omitted)
[  7]   1.00-2.00   sec   127 KBytes  1.04 Mbits/sec  90  (omitted)
[  7]   0.00-1.00   sec   129 KBytes  1.05 Mbits/sec  91
[  7]   1.00-2.00   sec   129 KBytes  1.05 Mbits/sec  91
[  7]   2.00-3.00   sec   127 KBytes  1.04 Mbits/sec  90
[  7]   3.00-4.00   sec   129 KBytes  1.05 Mbits/sec  91
[  7]   4.00-5.00   sec   127 KBytes  1.04 Mbits/sec  90
[  7]   5.00-6.00   sec   129 KBytes  1.05 Mbits/sec  91
[  7]   6.00-7.00   sec   127 KBytes  1.04 Mbits/sec  90
[  7]   7.00-8.00   sec   129 KBytes  1.05 Mbits/sec  91
[  7]   8.00-9.00   sec   127 KBytes  1.04 Mbits/sec  90
[  7]   9.00-10.00  sec   129 KBytes  1.05 Mbits/sec  91
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bitrate         Jitter    Lost/Total Datagrams
[  7]   0.00-10.00  sec  1.25 MBytes  1.05 Mbits/sec  0.000 ms  0/906 (0%)  sender
[  7]   0.00-10.00  sec  1.25 MBytes  1.05 Mbits/sec  0.013 ms  0/906 (0%)  receiver

iperf Done.

*/
