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
#include <QTextStream>

#include <ctime>      // For std::time_t, std::tm, std::time, std::localtime
#include <iomanip>    // For std::put_time
#include <sstream>    // For std::ostringstream

#include "../src/tpmgrdata.h"

IperfWrapper::IperfWrapper(bool ignorewronginterval, QObject *parent)
    : QObject{parent}, m_ignorewronginterval(ignorewronginterval)
{
    m_debuglv=3;
    m_restarttimeoffset = 0;
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

        mDuration = jsondata.value("duration", 0).toUInt();
        args = args + " -t " + QString::number(mDuration);

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
        debug("iTimestampLength:" + QString::number(iTimestampLength) + " timestamps:"
              + jsondata["timestamps"].toString(), 4);
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
    // mDuration = jsondata.value("duration", 0).toUInt();
    if (!bServer){
        bool bidir = jsondata["bidir"].toBool();
        if (bidir){
            args = args + " --dualtest "; // can not use with localhost/127.0.0.1
        }
        bool reverse = jsondata["reverse"].toBool();
        if (reverse){
            args = args + " -R ";
        }
        int duration = jsondata.value("duration", 0).toUInt();
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
    QString idx = "";
    QString sDir = TPDIRNO;

    if (linedata.startsWith("###")) {
        // TODO: handle custom value
    } else if (linedata.contains("Server listening") ||
               linedata.contains("Client connecting")) {
        if (linedata.contains("TCP")) {
            if (!m_protocal.contains("TCP")) {
                debug("wrong protocal setting:" + m_protocal + " actually:" + linedata, 3);
            }
            m_protocal = "TCP";
        } else if (linedata.contains("UDP")) {
            if (!m_protocal.contains("UDP")) {
                debug("wrong protocal setting:" + m_protocal + " actually:" + linedata, 3);
            }
            m_protocal = "UDP";
        } else {
            debug("unknown protocal??" + linedata, 2);
            return;
        }
        return;
    } else if (linedata.contains("--") || linedata.contains("Connecting") ||
        linedata.contains("Interval") ||
        linedata.contains("(omitted)") || linedata.contains("- - ") ||
        linedata.contains("Reverse mode") || linedata.contains("sender") ||
        linedata.contains("error") || linedata.contains("warning:")) {
        // Skip uninteresting lines
        debug("Skipping line: " + linedata, 4);
    } else if (linedata.contains("connected with")) {
        // Parse connection info and direction
        linedata = getIdx(linedata, idx);
        QStringList ds = linedata.split(" ", Qt::SkipEmptyParts);
        debug("ds:" + ds.join(",") + " length:" + QString::number(ds.length()), 4);

        bool isReverse = false;
        bool lastMatchesPort = false;
        if (ds.length() >= 9) {
            isReverse = linedata.contains("reverse");
            lastMatchesPort = ds[8].contains(QString::number(m_port));
        }
        if (m_bidir){
            if (!isReverse && !lastMatchesPort){
                sDir = m_bidirtag;
            }
        }else{
            if (!isReverse && !lastMatchesPort){
                sDir = TPDIRRx;
            }else{
                sDir = TPDIRTx;
            }
        }
        if (!sDir.isEmpty()){
            if (!m_idxdir.contains(idx)) {
                m_idxdir.insert(idx, sDir);
            } else {
                debug("THIS should not Happen!! update idx:" + idx + " dir to :" + sDir, 3);
            }
        }
    } else if (linedata.isEmpty()) {
            // ignore warning: line
    } else {
        // Handle throughput data
        linedata = getIdx(linedata, idx);
        if (m_idxdir.contains(idx)){
            sDir = m_idxdir.value(idx);
        } else {
            debug("//No need report:" + idx, 5);
            return;
        }
        QString sTag = m_servermode ? "s" : "c";
        int iparallel = m_parallel.toInt();
        QStringList data = linedata.split(" ", Qt::SkipEmptyParts);

        if (data.length() >= 6) {
            QString sInterval = data[0];
            QString chkInterval = "";
            double interval = 0.0;

            if (sInterval.contains("-")) {
                QStringList ds = sInterval.split("-");
                if (ds.length() == 2){
                    interval = ds[1].toDouble() - ds[0].toDouble();
                    chkInterval = ds[1];
                    debug("s:"+ds[0]+ " e:" + ds[1] + " mDuration:" +QString::number(mDuration));
                    if ((ds[0].toDouble() == 0)&&
                        (qAbs(ds[1].toDouble() - mDuration) < 1e-9)
                        ){
                        debug("iperf2 reach end of test: " + linedata);
                    }
                }
            }

            if (m_ignorewronginterval && !linedata.contains("receiver") &&
                qAbs(m_interval - interval) > 0.5) {
                debug("Ignore wrong interval: " + QString::number(interval), 4);
                return;
            }

            if (m_omit > 0) {
                double dInterval = chkInterval.toDouble() - m_omit;
                if (dInterval <= 0) {
                    debug("Ignore omit time: " + QString::number(dInterval), 2);
                    return;
                }
            }

            if (!m_tpdatas.contains(sInterval))
                m_tpdatas.insert(sInterval, QJsonArray());

            if (m_tpdatas[sInterval].count() < iparallel) {
                if (qAbs(m_interval - interval) > 0.5) {
                    debug("Skip interval mismatch: " + QString::number(interval), 3);
                } else {
                    QJsonObject irec;
                    irec.insert("idx", QString("%1%2").arg(idx, sTag));
                    irec.insert("interval", interval);
                    irec.insert("value", data[4]);
                    irec.insert("unit", data[5]);
                    if (m_protocal.contains("UDP") && data.length() >= 9) {
                        irec.insert("jitter", data[6]);
                        irec.insert("jitter_unit", data[7]);
                        QStringList pkts = data[8].split("/");
                        if (pkts.length() == 2) {
                            irec.insert("packet_lost", pkts[0]);
                            irec.insert("packet_total", pkts[1]);
                        } else {
                            debug("Malformed packet lost field: " + data[8], 3);
                        }
                    }
                    if (!sDir.isEmpty()){
                        irec.insert("dir", sDir);
                    }

                    if (!idx.contains("SUM", Qt::CaseInsensitive)) {
                        if (linedata.contains("receiver"))
                            irec.insert("AVG", true);
                        m_tpdatas[sInterval].append(irec);
                    } else {
                        debug("Ignoring [SUM] line: " + linedata, 5);
                    }
                }
            }

            if (m_tpdatas[sInterval].count() >= iparallel &&
                !idx.contains("SUM", Qt::CaseInsensitive)) {
                QJsonArray arr = m_tpdatas[sInterval];
                QJsonDocument doc;
                doc.setArray(arr);

                if (sInterval.contains("-")) {
                    QStringList ls = sInterval.split("-");
                    if (ls.length() == 2)
                        sInterval = ls[1];
                    else
                        debug("Unrecognized interval format: " + sInterval, 3);
                }
                if (m_omit > 0) {
                    sInterval = QString::number(sInterval.toDouble() - m_omit);
                }
                if (m_delaytime > 0) {
                    sInterval = QString::number(sInterval.toDouble() + m_delaytime);
                }
                if (m_restarttimeoffset > 0){
                    sInterval = QString::number(sInterval.toDouble() + m_restarttimeoffset);
                }
                QString tpdata = doc.toJson(QJsonDocument::Compact);
                // qDebug() << "sInterval: " << sInterval << " tpdata:" << tpdata;
                emit sendThroughput(m_refrow, sInterval, tpdata);
                m_tpdatas.remove(sInterval);
            }
        } else {
            debug("parserIperf2: unknown format of line: " + linedata, 2);
        }
    }
}

void IperfWrapper::parserIperf3(QString linedata)
{
    // try{
        QString idx = "";
        if (bWithtimestamp){
            linedata = linedata.mid(iTimestampLength);
            debug("after trim time stamp:" + linedata, 3);
        }
        if (linedata.startsWith("###")){
            //TODO: handle custom value
/*
### /tmp/qiperf/iperf3 -s -p 5201 --bind 127.0.0.1 -i 1 --one-off -f m --forceflush
### 2025-06-27_133520.434
*/
        }else if (linedata.contains("Server listening")||
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
            debug("TODO: error message: " + linedata, 3);
        }else if(linedata.contains("warning:")){
            // ignore warning: line
        }else if (linedata.isEmpty()) {
            // ignore warning: line
        }else{
            QString sDir = "";
            linedata = getIdx(linedata, idx);
            QString sTag="";
            if (m_servermode){
                sTag="s";
            }else{
                sTag="c";
            }
            int iparallel = m_parallel.toInt();
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
                    // debug("ignore m_bidir part data: " + sDir, 3);
                    return;
                }
            }else{
                sDir = m_bidirtag;
            }
            if (sDir.isEmpty()){
                debug("Ignore Not sDir line data:" + linedata, 1);
                return;
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
                        // expect as(--interval) -i <num>
                        interval = ds[1].toDouble() - ds[0].toDouble();
                    }
                }
                if (m_ignorewronginterval){
                    //if (!linedata.contains("receiver")) // very last recored may be not correct data
                    {
                        double diffint = qAbs(m_interval-interval);
                        QString msg = m_ignorewronginterval?"true":"false";
                        debug("m_ignorewronginterval:" + msg+
                              " diffint:" + QString::number(diffint), 5);
                        // check report interval value is correct (smallest value 1 sec)
                        if (diffint>0.0){
                            //interval value not match value of -i (--interval)
                            debug(linedata + " =>>>>parserIperf3 ignorewronginterval value:" +
                                  QString::number(interval) + " expect:" + QString::number(m_interval), 4);
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

                    // debug("("+sInterval + ") " + idx + sTag +
                    //       " interval:" + QString::number(interval) +
                    //       " value:" + data[4] +
                    //       " sDIR:" +  sDir);

                    irec.insert("value", data[4]);  // Bitrate
                    irec.insert("unit", data[5]);  // Bitrate unit
                    if (m_protocal.contains("UDP")){
                        if (data.length() >=9) {
                            irec.insert("jitter", data[6]);
                            irec.insert("jitter_unit", data[7]);
                            QStringList pkts = data[8].split("/");
                            if (pkts.count()==2){
                                irec.insert("packet_lost", pkts[0]);
                                irec.insert("packet_total", pkts[1]);
                            }else{
                                debug("Unknown data format of packet lost: " + data[8], 3);
                            }
                        }else{
                            debug("Unknown data format: " + data.join(","), 3);
                        }
                    }
                    if (!sDir.isNull()){
                        irec.insert("dir", sDir);  // direction
                    }
                    if (idx.contains("SUM", Qt::CaseInsensitive)){
                        // ignore [SUM] line
                        debug("==linedata==SUM==  " + linedata, 5);
                    }else{
                        if (linedata.contains("receiver")){
                            //final data is the average of throughput
                            irec.insert("AVG", true);
                        }
                        m_tpdatas[sInterval].append(irec);
                    }
                }
                //else {
                //     m_tpdatas[sInterval].count();
                // }
                if ((m_tpdatas[sInterval].count()>=iparallel)&&
                     !idx.contains("SUM", Qt::CaseInsensitive)){
                    QJsonArray arr = m_tpdatas[sInterval];
                    debug("sInterval:" + sInterval + " m_tpdatas:" + QString::number(arr.size()), 4);
                    QJsonDocument doc;
                    doc.setArray(arr);
                    if (sInterval.contains("-")){
                        QStringList ls_int = sInterval.split("-");
                        if (ls_int.length()==2){
                            sInterval = ls_int[1];
                        }else{
                            debug("unknown format of sInterval: " + sInterval, 3);
                        }
                    }
                    if (m_delaytime>0){
                        sInterval = QString::number(sInterval.toDouble()+ m_delaytime);
                    }
                    if (m_restarttimeoffset>0){
                        sInterval = QString::number(sInterval.toDouble()+ m_restarttimeoffset);
                    }
                    //iperf3 throughput data
                    emit sendThroughput(m_refrow, sInterval, doc.toJson(QJsonDocument::Compact));
                    //clear record
                    m_tpdatas.remove(sInterval);
                }else{
                    // debug("m_tpdatas:" + m_tpdatas);//when many data, this cause crash?
                }
            } else {
                debug("parserIperf3: unknown format of line: " + linedata, 2);
            }
        }
    // }catch (const std::exception &e) {
    //     // Handle the exception and show an error message
    //     debug("IperfWrapper::parserIperf3 Exception Caught" + QString(e.what()), 3);
    // }catch (...){
    //     debug("IperfWrapper::parserIperf3 Unknown ERROR", 3);
    // }
}

QString IperfWrapper::getIdx(QString linedata, QString &idx)
{
    QString result = linedata;
    int iS = linedata.indexOf("[",0, Qt::CaseInsensitive);
    int iE = linedata.indexOf("]",0, Qt::CaseInsensitive);
    QString tmp = linedata.mid(iS,iE-iS+1).trimmed();  // extract [ idx]
    if (!tmp.isEmpty()){
        //get number only
        static const QRegularExpression re(R"(\[\s*\*?\s*(\d+)\s*\])");
        QRegularExpressionMatch match = re.match(tmp);
        if (match.hasMatch()) {
            idx = match.captured(1);
            result = linedata.right(linedata.length()-iE-1).trimmed();
            debug("getIdx idx: " + idx +  "  right(" + result + ")", 5);
        }else{
            debug("getIdx format not in expect eq [ 1]: " + tmp, 5);
        }
    }
    return result;
}

void IperfWrapper::setSetting(int refrow, bool servermode, QString parallel, bool bidir, QString bidirtag)
{
    m_refrow = refrow;
    m_servermode = servermode;
    m_parallel = parallel;
    m_bidir = bidir;
    m_bidirtag = bidirtag;
    debug("setSetting m_bidirtag:" + m_bidirtag, 5);
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

void IperfWrapper::setDuration(uint duration)
{
    debug("setDuration:"+ QString::number(duration));
    mDuration = duration;
}

void IperfWrapper::setDelaytime(int delaytime)
{
    m_delaytime = delaytime;
}

void IperfWrapper::setRestarttimeoffset(qint64 restarttimeoffset)
{
    m_restarttimeoffset = restarttimeoffset;
}

void IperfWrapper::setInterval(uint interval)
{
    m_interval = interval;
}

void IperfWrapper::setArgs(QString arg)
{
#if QT_VERSION < 0x050E00 // < 5.14.0
    m_arguments = arg.split(" ", QString::SkipEmptyParts);
#else
    m_arguments = arg.split(" ", Qt::SkipEmptyParts);
#endif
    debug("m_arguments:" + m_arguments.join(","), 4);
    if (m_arguments.contains("timestamps")){
        bWithtimestamp = true;
        QRegularExpression regex("^" + QRegularExpression::escape("--timestamps"),
                                 QRegularExpression::CaseInsensitiveOption); // Case-insensitive search

        QStringList filteredList = m_arguments.filter(regex);
        debug("filteredList:" + filteredList.join(","), 3);
        // int idx = m_arguments.indexOf("--timestamps");
        // qDebug() << "timestamps idx = " << QString::number(idx);
        //iTimestampLength = getTimeStempLength(jsondata["timestamps"].toString().toStdString());
    }
}

void IperfWrapper::setOmit(int omit)
{
    m_omit = omit;
}

void IperfWrapper::debug(QString msg, int debuglv)
{
    if (debuglv <= m_debuglv){
        emit debuginfo(msg);
    }
}

void IperfWrapper::setDebugLevel(int level)
{
    m_debuglv = level;
}

void IperfWrapper::work()
{
    //log run
    QFile file(m_filename);
    if(file.exists()){
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
                    debug("[IperfWrapper::work]: Not support iperf version:" +m_version, 3);
                    break;
                }
            }
            file.close();
            emit progress(m_filename, -1);
        }else{
            debug("open file " + m_filename + " Fail!!", 3);
        }
    }else{
        debug("file not exist: " + m_filename, 3);
    }
    emit workFinished();
}

// int IperfWrapper::getTimeStempLength(QString timestempformat)
qint64 IperfWrapper::getTimeStempLength(const std::string& format_string)
{
    //timestemp in
    // %Y = Year (2025)
    // %y = Year in 2 decimal number (2025 = 25)
    // %m = Month (01-12)
    // %d = Day of the month as a decimal number (01-31)
    // %H = Hour (24-hour clock) as a decimal number (00-23)
    // %M = Minute as a decimal number (00-59)
    // %S = Second as a decimal number (00-60)
    // %F: Equivalent to %Y-%m-%d
    // %T: Equivalent to %H:%M:%S
    // For production, you might pass a specific std::tm or std::time_t.
    std::ostringstream oss;

    std::time_t now = std::time(nullptr);
#if defined(Q_OS_LINUX)
    std::tm* local_tm = std::localtime(&now);
    if (local_tm == nullptr) {
        debug("Error: Could not get local time.", 3);
        return 0; // Or throw an exception
    }
    oss << std::put_time(local_tm, format_string.c_str());

#endif
#if defined(Q_OS_WIN)
    // Declare a std::tm object that localtime_s will populate
    std::tm local_tm;
    errno_t err = localtime_s(&local_tm, &now);

    if (err == 0) {
        // Successfully converted the time
        oss << std::put_time(&local_tm, format_string.c_str());
#endif
        // Check for errors during formatting (e.g., invalid format string)
        if (oss.fail()) {
            debug("Error: Failed to format time with format string: \"" + QString::fromStdString(format_string) + "\"", 3);
            return 0; // Or throw an exception
        }
#if defined(Q_OS_WIN)
    }
#endif
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
