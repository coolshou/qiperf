#include "iperfwrapper.h"

// #include <QCoreApplication>
// #include <QEventLoop>
#include <QVariantMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QThread>
#include "../src/tpmgrdata.h"

#include <QDebug>

IperfWrapper::IperfWrapper(bool ignorewronginterval, QObject *parent)
    : QObject{parent}, m_ignorewronginterval(ignorewronginterval)
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
        bool zerocopy = jsondata["zerocopy"].toBool();
        if (zerocopy){
            args = args + " -Z";
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
        }else{
            if (protocal.contains("UDP")){
                args = args + " -b 0 ";
            }
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
    try{
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
            // when see this : iperf server run finish!!
        }else if(linedata.contains("sender")){
            // ignore sender
        }else{
    //        qDebug() << "parserIperf3: " << linedata;
            QString sDir = nullptr;
            int iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
            QString idx = linedata.mid(1,iS-1).trimmed();  // extract [ idx]
            linedata = linedata.right(linedata.length()-iS-1);
            QString sTag="";
            if (m_servermode){
                sTag="s";
            }else{
                sTag="c";
            }
            int iparallel = m_parallel.toInt();
    //        qDebug() << "iparallel: " << QString::number(iparallel);
            if (m_bidir){
                //bidir mode
                // in bidir only get [Rx*]
                iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
                sDir = linedata.mid(1,iS-1).trimmed();// server:[TX-S][RX-S], client:[TX-C][RX-C]
                linedata = linedata.right(linedata.length()-iS-1);
                if (sDir.contains(TPDIRRx, Qt::CaseSensitivity::CaseInsensitive)){
                    if (m_servermode){
                        sDir = TPDIRRx;
                    }else{
                        sDir = TPDIRTx;
                    }
                    linedata = linedata.trimmed();
                }else {
                    // qInfo() << "ignore m_bidir part data: " << sDir;
                    return;
                }
            }else{
                sDir = m_bidirtag;
            }
            // TCP:
            //"0.00-1.00   sec   111 MBytes   931 Mbits/sec"
            // UDP:
            // 0.00-1.00   sec  4.18 GBytes  35.9 Gbits/sec  0.001 ms  0/137110 (0%)
#if QT_VERSION < 0x050E00 // < 5.14.0
            QStringList data = linedata.split(" ", QString::SkipEmptyParts);
#else
            QStringList data = linedata.split(" ", Qt::SkipEmptyParts); // qt 5.14
#endif
            if (data.length()>=6){
                QString sInterval  = data[0]; // Interval
                double interval = 0.0;
                if (sInterval.contains("-")){
                    QStringList ds = sInterval.split("-");
                    if (ds.length()==2){
                        interval = ds[1].toDouble() - ds[0].toDouble();
                        // qDebug() << "ds[1]:" << ds[1] << " ds[0]:" << ds[0] << "====interval:" << QString::number(interval);
                    }
                }
                if (m_ignorewronginterval){
                    if (!linedata.contains("receiver")){
                        // check report interval value is correct (smallest value 1 sec)
                        if (qAbs(m_interval-interval)>0.5){
                            qInfo() << linedata << "=>>>> ignorewronginterval value:" << QString::number(interval) << " expect:" << QString::number(m_interval);
                            return;
                        }
                    }
                }

                if (!m_tpdatas.contains(sInterval)){
                    QJsonArray lst =QJsonArray();
                    m_tpdatas.insert(sInterval, lst);
                }
                if (m_tpdatas[sInterval].count()<iparallel){
                    QJsonObject irec = QJsonObject();
                    irec.insert("idx", idx+sTag);  // parallel num
                    irec.insert("interval", interval);  // interval
                    irec.insert("value", data[4]);  // Bitrate
                    irec.insert("unit", data[5]);  // Bitrate unit
                    if (m_protocal.contains("UDP")){
                        if (data.length() >=9) {
                            irec.insert("jitter", data[6]);
                            irec.insert("jitter_unit", data[7]);
                            QStringList pkts = data[8].split("/");
                            if (pkts.count()==2){
        //                        qInfo() << "packet_lost/packet_total = " << pkts[0] << " / " << pkts[1];
                                irec.insert("packet_lost", pkts[0]);
                                irec.insert("packet_total", pkts[1]);
                            }else{
                                qDebug() << "Unknown data format of packet lost: " << data[8];
                            }
                        }else{
                            //qDebug() << "Unknown data format: " << data;
                        }
                    }
                    if (!sDir.isNull()){
                        irec.insert("dir", sDir);  // direction
                    }
                    if (idx.contains("SUM", Qt::CaseInsensitive)){
                        // ignore [SUM] line
                        qInfo() << "==linedata==SUM==  " << linedata;
                    }else{
        //                qDebug() << "m_parallel: " << QString::number(iparallel) << "m_tpdatas length: " << m_tpdatas[sInterval].count();
                        if (linedata.contains("receiver")){
                            irec.insert("AVG", true); //final data is the average of throughput
                        }
                        m_tpdatas[sInterval].append(irec);
                    }
                }
                if ((m_tpdatas[sInterval].count()>=iparallel)&&
                     !idx.contains("SUM", Qt::CaseInsensitive)){
                    QJsonArray arr = m_tpdatas[sInterval];
                    QJsonDocument doc;
                    doc.setArray(arr);
                    if (sInterval.contains("-")){
                        QStringList ls_int = sInterval.split("-");
                        if (ls_int.length()==2){
                            sInterval = ls_int[1];
                        }else{
                            qDebug() << "unknown format of sInterval: " << sInterval;
                        }
                    }
    //                qDebug() << "sInterval:" << sInterval << " doc:" ;
                    if (m_delaytime>0){
                        sInterval = QString::number(sInterval.toDouble()+ m_delaytime);
                    }
                    emit sendThroughput(m_idx, sInterval, doc.toJson(QJsonDocument::Compact));
                    //clear record
                    m_tpdatas.remove(sInterval);
                }
            } else {
                qDebug() << "parserIperf3: unknown format of line: " << linedata;
            }
        }
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        qDebug() << "IperfWorker::work Exception Caught" << e.what();
    }catch (...){
        qDebug() << "IperfWorker::work Unknown ERROR";
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

void IperfWrapper::setFile(QString filename)
{
    m_filename = filename;
}

void IperfWrapper::setIperf(QString version, QString protocal)
{
    m_version = version;
    m_protocal = protocal;
}

void IperfWrapper::setDelaytime(int delaytime)
{
    m_delaytime = delaytime;
}

void IperfWrapper::setInterval(uint interval)
{
    m_interval = interval;
}

void IperfWrapper::work()
{
    //log run
    QFile file(m_filename);
    if(file.exists()){
//        qDebug() <<"start IperfWrapper::work: " << m_filename ;
        if (file.open(QIODevice::ReadOnly)){
            QTextStream in(&file);
            int lineNumber = 0;
            while (!in.atEnd())
            {
                QString line = in.readLine();
                lineNumber++;
                emit progress(m_filename, lineNumber);
                if (m_version=="3"){
                    if (line!=""){
                        parserIperf3(line);
                    }
                }else if (m_version=="2"){
                    qDebug() << "[IperfWrapper::work]TODO: parser iperf2 output";
                }else {
                    qDebug() << "[IperfWrapper::work]: Not support iperf version:" <<m_version;
                }
                // QCoreApplication::processEvents(QEventLoop::AllEvents);
//                QThread::msleep(18);// slow down to avoid app crash under windows
            }
            file.close();
//            qDebug() << "finish file parser: " << m_filename;
            emit progress(m_filename, -1);
        }else{
            qDebug() << "open file " << m_filename << " Fail!!";
        }
    }else{
        qDebug() << "file not exist: " << m_filename;
    }
    emit workFinished();
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
