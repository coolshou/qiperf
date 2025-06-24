#include "iperfwrapper.h"

// #include <QCoreApplication>
// #include <QEventLoop>
#include <QVariantMap>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QThread>
#include <QRegularExpression>

#include <ctime>      // For std::time_t, std::tm, std::time, std::localtime
#include <iomanip>    // For std::put_time
#include <sstream>    // For std::ostringstream

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
        int duration = jsondata["duration"].toInt();
        if (duration>=0){
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
            // force UDP use Max throughput, default 1 Mbit/sec for UDP
            if (protocal.contains("UDP")){
                args = args + " -b 0 ";
            }
        }
        uint buffer = jsondata["buffer"].toUInt();
        if (buffer>0){
            QString unit_buffer = jsondata["unit_buffer"].toString();
            args = args + " -l " +QString::number(buffer)+ unit_buffer;
        }
        int dscp = jsondata["dscp"].toInt();
        if ((dscp>=0)&&(dscp<=64)){
            args = args + " --dscp " +QString::number(dscp);
        }
        uint mss = jsondata["mss"].toUInt();
        if (mss>0){
            args = args + " -M " +QString::number(mss);
        }
        int tos = jsondata["tos"].toInt();
        if ((tos>=0)&&(tos<=255)){
            args = args + " --tos " +QString::number(tos);
        }
    }else {
        args = args +" --one-off ";
    }
    QString fmtreport = jsondata["fmtreport"].toString();
    if (!fmtreport.isEmpty()){
        args = args + " -f " + fmtreport;
    }
    if (jsondata.contains("timestamps")){
        bWithtimestamp = true;
        iTimestampLength = getTimeStempLength(jsondata["timestamps"].toString().toStdString());
        qDebug()<< "iTimestampLength:" << QString::number(iTimestampLength) << " timestamps:" << jsondata["timestamps"].toString();
        args = args + " --timestamps=" + jsondata["timestamps"].toString();
    }
    args = args + " --forceflush ";
    return args;
}

QString IperfWrapper::toIperf2args(QVariantMap jsondata)
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
    QString target = "127.0.0.1";
    if (!bServer){
        target = jsondata["target"].toString();
        if (!target.isEmpty()){
            args = args + " -c " + target;
        }
    }
    QString bindaddr = jsondata["bind"].toString();
    if (!bindaddr.isEmpty()){
        args = args + " -B " + bindaddr;
    }
    if (!bServer){
        bool bidir = jsondata["bidir"].toBool();
        if (bidir){
            args = args + " --dualtest "; // can not use with localhost/127.0.0.1
        }
        bool reverse = jsondata["reverse"].toBool();
        if (reverse){
            args = args + " -R ";
        }
        int duration = jsondata["duration"].toInt();
        if (duration>=0){
            args = args + " -t " + QString::number(duration);
        }
        // bool zerocopy = jsondata["zerocopy"].toBool();
        // if (zerocopy){
        //     args = args + " -Z";
        // }
    }
    uint interval = jsondata["interval"].toUInt();
    if (interval>0){
        args = args + " -i " + QString::number(interval);
    }
    // iperf2 need set -u at both server and client
    QString protocal = jsondata["protocal"].toString();
    if (protocal.contains("UDP")){
        args = args + " -u ";
    }else{
        uint omit = jsondata["omit"].toUInt();
        if (omit>0){
            args = args + " --omit " + QString::number(omit);
        }
    }
    uint windowsize = jsondata["windowsize"].toUInt();
    if (windowsize>0){
        QString unit_windowsize = jsondata["unit_windowsize"].toString();
        args = args + " -w " +QString::number(windowsize)+ unit_windowsize;
    }
    if (!bServer){
        // uint omit = jsondata["omit"].toUInt();
        // if (omit>0){
        //     args = args + " --txdelay-time " + QString::number(omit);
        // }
        uint parallel = jsondata["parallel"].toUInt();
        if (parallel>1){
            args = args + " -P " + QString::number(parallel);
        }

        uint bitrate = jsondata["bitrate"].toUInt();
        if (bitrate>0){
            QString unit_bitrate = jsondata["unit_bitrate"].toString();
            args = args + " -b " +QString::number(bitrate)+ unit_bitrate;
        }else{
            // force UDP use Max throughput, default 1 Mbit/sec for UDP
            if (protocal.contains("UDP")){
                args = args + " -b -1 "; // iperf2 UDP with -R can not use -b 0 for Max throughput, use -1 instead
            }
        }
        uint buffer = jsondata["buffer"].toUInt();
        if (buffer>0){
            QString unit_buffer = jsondata["unit_buffer"].toString();
            args = args + " -l " +QString::number(buffer)+ unit_buffer;
        }
        if (jsondata.contains("dscp")){
            int dscp = jsondata["dscp"].toInt();
            if ((dscp>=0)&&(dscp<=64)){
                args = args + " --dscp " +QString::number(dscp);
            }
        }
        uint mss = jsondata["mss"].toUInt();
        if (mss>0){
            args = args + " -M " +QString::number(mss);
        }
    }
    if (jsondata.contains("tos")){
        int tos = jsondata["tos"].toInt();
        if ((tos>=0)&&(tos<=255)){
            args = args + " -S " +QString::number(tos);
        }
    }
    QString fmtreport = jsondata["fmtreport"].toString();
    if (!fmtreport.isEmpty()){
        args = args + " -f " + fmtreport;
    }
    return args;
}

void IperfWrapper::parserIperf2(QString linedata)
{
    try{
        QString idx = "";
        QString sDir = TPDIRNO;
        if (linedata.contains("Server listening")||
            linedata.contains("Client connecting")){
            if (linedata.contains("TCP")){
                if(!m_protocal.contains("TCP")){
                    qDebug() << "wrong protocal setting:" << m_protocal << " actually:" << linedata;
                }
                m_protocal = "TCP";
            }else if (linedata.contains("UDP")){
                if(!m_protocal.contains("UDP")){
                    qDebug() << "wrong protocal setting:" << m_protocal << " actually:" << linedata;
                }
                m_protocal = "UDP";
            }else{
                qDebug() << "unknown protocal??" << linedata;
                return;
            }
        }else if (linedata.contains("--")||
            linedata.contains("Connecting")||
            linedata.contains("local") ||
            linedata.contains("Interval") ||
            linedata.contains("(omitted)")||
            linedata.contains("- - ")||
            linedata.contains("Reverse mode")){
            //ignore this lines
        }else if (linedata.contains("connected with")){
            // collect idx? and each idx's direction
            //[  1] local 192.168.70.147 port 37232 connected with 192.168.70.14 port 5001
            //[  2] local 192.168.70.147 port 5001 connected with 192.168.70.14 port 49132
            linedata = getIdx(linedata,idx);
            QStringList ds;
#if QT_VERSION < 0x050E00 // < 5.14.0
            ds = linedata.split(" ", QString::SkipEmptyParts);
#else
            ds = linedata.split(" ", Qt::SkipEmptyParts); // qt 5.14
#endif
            qDebug() << "ds:" << ds << " length:" << ds.length();
            if(ds.length()>=9){
                if (linedata.contains("reverse")){
                    if (ds.last().contains(QString::number(m_port))){
                        sDir = TPDIRRx;
                    }else{
                        sDir = TPDIRTx;
                    }
                }else{
                    if (ds.last().contains(QString::number(m_port))){
                        sDir = TPDIRTx;
                    }else{
                        sDir = TPDIRRx;
                    }
                }
            }
            if (!m_idxdir.contains(idx)){
                qDebug() << "Record : " << idx << " = " << sDir;
                m_idxdir.insert(idx, sDir);
            }else{
                qDebug() << "THIS should not Happen!! update idx:" << idx << " dir to :" << sDir;
            }
        }else if(linedata.contains("TCP window size")){
            // TODO : TCP window size?
        }else if(linedata.contains("iperf Done.")){
            // when see this : iperf server run finish!!
        }else if(linedata.contains("sender")){
            // ignore sender
        }else if(linedata.contains("error")){
            // ignore error line
            qDebug() << "TODO: error message: " << linedata;
        }else if(linedata.contains("warning:")){
            // ignore warning: line
        }else{
            linedata = getIdx(linedata, idx);
            // qDebug() << "parserIperf2 after idx: " << idx << " linedata: " << linedata;
            QString sTag="";
            if (m_servermode){
                sTag="s";
            }else{
                sTag="c";
            }
            // qDebug() << "m_idxdir:" << QString::number(m_idxdir.keys().length());
            int iparallel = m_parallel.toInt();
            //        qDebug() << "iparallel: " << QString::number(iparallel);
            // if (m_bidir){
            //     //bidir mode
            //     // in bidir only get [Rx*]
            //     iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
            //     sDir = linedata.mid(1,iS-1).trimmed();// server:[TX-S][RX-S], client:[TX-C][RX-C]
            //     linedata = linedata.right(linedata.length()-iS-1);
            //     if (sDir.contains(TPDIRRx, Qt::CaseSensitivity::CaseInsensitive)){
            //         if (m_servermode){
            //             sDir = TPDIRRx;
            //         }else{
            //             sDir = TPDIRTx;
            //         }
            //         linedata = linedata.trimmed();
            //     }else {
            //         // qInfo() << "ignore m_bidir part data: " << sDir;
            //         return;
            //     }
            // }else{
            //     sDir = m_bidirtag;
            // }
            // TCP:
            //"0.0000-1.0000 sec  4595 MBytes  38548 Mbits/sec"
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
                if (m_omit>0){
                    qInfo() << "iperf2 precess m_omit:" << QString::number(m_omit);
                    int dInterval = sInterval.toInt() - m_omit;
                    if (dInterval<=0){
                        qInfo() << "iperf2 ignore omit time";
                        return;
                    }
                    sInterval = QString::number(dInterval);
                }
                if (!m_tpdatas.contains(sInterval)){
                    QJsonArray lst =QJsonArray();
                    m_tpdatas.insert(sInterval, lst);
                }
                // qDebug() << "sInterval: " << sInterval << " m_tpdatas[sInterval].count(): " << m_tpdatas[sInterval].count();
                if (m_tpdatas[sInterval].count()<iparallel){
                    // qDebug() << "m_interval: " << QString::number(m_interval)
                    //          << " interval:" << QString::number(interval);

                    if (qAbs(m_interval-interval)>0.5){
                        qDebug() << " skip this interval:" << QString::number(interval);
                    }else{
                        QJsonObject irec = QJsonObject();
                        irec.insert("idx", QString("%1%2").arg(idx,sTag));  // parallel num
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
                            // qDebug() << "sInterval: " << sInterval << " append:" << irec;
                            m_tpdatas[sInterval].append(irec);
                        }
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
                    // qInfo() << "sendThroughput sInterval-key:" << sInterval << " data: " << doc.toJson(QJsonDocument::Compact);
                    emit sendThroughput(m_idx, sInterval, doc.toJson(QJsonDocument::Compact));
                    //clear record
                    m_tpdatas.remove(sInterval);
                }
            } else {
                qDebug() << "parserIperf2: unknown format of line: " << linedata;
            }
        }
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        qDebug() << "parserIperf2::work Exception Caught" << e.what();
    }catch (...){
        qDebug() << "parserIperf2::work Unknown ERROR";
    }
}

void IperfWrapper::parserIperf3(QString linedata)
{
    try{
        QString idx = "";
        if (bWithtimestamp){
            linedata = linedata.mid(iTimestampLength);
            qDebug() << "after trim time stamp:" << linedata;
        }
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
        }else if(linedata.contains("error")){
            // ignore error line
            qDebug() << "TODO: error message: " << linedata;
        }else if(linedata.contains("warning:")){
            // ignore warning: line
        }else{
    //        qDebug() << "parserIperf3: " << linedata;
            QString sDir = nullptr;
            linedata = getIdx(linedata, idx);
            // qint64 iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
            // QString idx = linedata.mid(1,iS-1).trimmed();  // extract [ idx]
            // linedata = linedata.right(linedata.length()-iS-1);
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
                int iS = linedata.indexOf("]",0, Qt::CaseInsensitive);
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
                    irec.insert("idx", QString("%1%2").arg(idx,sTag));  // parallel num
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
                }else{
                    qDebug() << "m_tpdatas:" << m_tpdatas;
                }
            } else {
                qDebug() << "parserIperf3: unknown format of line: " << linedata;
            }
        }
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        qDebug() << "IperfWrapper::parserIperf3 Exception Caught" << e.what();
    }catch (...){
        qDebug() << "IperfWrapper::parserIperf3 Unknown ERROR";
    }
}

QString IperfWrapper::getIdx(QString linedata, QString &idx)
{
    int iS = linedata.indexOf("[",0, Qt::CaseInsensitive);
    int iE = linedata.indexOf("]",0, Qt::CaseInsensitive);
    idx = linedata.mid(iS+1,iE-iS-1).trimmed();  // extract [ idx]
    QString r = linedata.right(linedata.length()-iE-1).trimmed();
    qDebug() << "getIdx idx: " << idx <<  "  right(" << r << ")";
    return r;
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

void IperfWrapper::setIperf(QString version, QString protocal, uint port)
{
    m_version = version;
    m_protocal = protocal;
    m_port = port;
}

void IperfWrapper::setDelaytime(int delaytime)
{
    m_delaytime = delaytime;
}

void IperfWrapper::setInterval(uint interval)
{
    // qDebug() << " setInterval:" << interval;
    m_interval = interval;
}

void IperfWrapper::setArgs(QString arg)
{
#if QT_VERSION < 0x050E00 // < 5.14.0
    m_arguments = arg.split(" ", QString::SkipEmptyParts);
#else
    m_arguments = arg.split(" ", Qt::SkipEmptyParts);
#endif
    qDebug() << "m_arguments:" << m_arguments;
    if (m_arguments.contains("timestamps")){
        bWithtimestamp = true;
        QRegularExpression regex("^" + QRegularExpression::escape("--timestamps"),
                                 QRegularExpression::CaseInsensitiveOption); // Case-insensitive search

        QStringList filteredList = m_arguments.filter(regex);
        qDebug() << "filteredList:" << filteredList;
        // int idx = m_arguments.indexOf("--timestamps");
        // qDebug() << "timestamps idx = " << QString::number(idx);
        //iTimestampLength = getTimeStempLength(jsondata["timestamps"].toString().toStdString());
    }
}

void IperfWrapper::setOmit(int omit)
{
    m_omit = omit;
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
                    if (line!=""){
                        parserIperf2(line);
                    }
                }else {
                    qDebug() << "[IperfWrapper::work]: Not support iperf version:" <<m_version;
                    break;
                }
            }
            file.close();
            emit progress(m_filename, -1);
        }else{
            qDebug() << "open file " << m_filename << " Fail!!";
        }
    }else{
        qDebug() << "file not exist: " << m_filename;
    }
    emit workFinished();
}

// int IperfWrapper::getTimeStempLength(QString timestempformat)
int IperfWrapper::getTimeStempLength(const std::string& format_string)
{
    //timestemp in %Y = 2025 %m = Month (01-12) %d = Day of the month as a decimal number (01-31)
    // %H = Hour (24-hour clock) as a decimal number (00-23)
    // %M = Minute as a decimal number (00-59)
    // %S = Second as a decimal number (00-60)
    // %F: Equivalent to %Y-%m-%d
    // %T: Equivalent to %H:%M:%S
    // For production, you might pass a specific std::tm or std::time_t.
    std::time_t now = std::time(nullptr);
    std::tm* local_tm = std::localtime(&now);

    if (local_tm == nullptr) {
        qDebug() << "Error: Could not get local time.";
        return 0; // Or throw an exception
    }

    std::ostringstream oss;
    oss << std::put_time(local_tm, format_string.c_str());

    // Check for errors during formatting (e.g., invalid format string)
    if (oss.fail()) {
        qDebug() <<  "Error: Failed to format time with format string: \"" << QString::fromStdString(format_string) << "\"";
        return 0; // Or throw an exception
    }

    return oss.str().length();


    /*
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // Format the QDateTime into a QString using the provided formatString
    QString formattedTime = currentDateTime.toString(timestempformat);

    // Check if the formatting was successful (though toString rarely fails for valid formats)
    if (formattedTime.isEmpty() && !timestempformat.isEmpty()) {
        qWarning() << "Warning: Formatting resulted in an empty string for format:" << timestempformat;
        return 0; // Or handle error appropriately
    }

    return formattedTime.length();
*/
}
