#include "iperffileworker.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

IperfFileWorker::IperfFileWorker(QString version, QString protocal,
                                 int idx, bool servermode, int parallel,
                                 bool bidir, QString bidirtag , QString filename,
                                 QObject *parent)
    : QObject{parent}, m_version(version), m_protocal(protocal),
      m_idx(idx), m_servermode(servermode), m_parallel(parallel),
      m_bidir(bidir), m_bidirtag(bidirtag), m_filename(filename)
{
    m_thread = new QThread();
    m_iperfwrapper= new IperfWrapper();
    m_iperfwrapper->setSetting(idx, servermode, QString::number(parallel), bidir, bidirtag);
    m_iperfwrapper->setFile(filename);
    m_iperfwrapper->setIperf(version, protocal);
    connect(m_iperfwrapper, &IperfWrapper::sendThroughput, this, &IperfFileWorker::onThroughputData);
    connect(m_iperfwrapper, &IperfWrapper::progress, this, &IperfFileWorker::onProgress);
    connect(m_thread, &QThread::started, m_iperfwrapper, &IperfWrapper::work);
    m_iperfwrapper->moveToThread(m_thread);
    connect(m_iperfwrapper, &IperfWrapper::workFinished, this, &IperfFileWorker::onWorkFinished);
    connect(m_iperfwrapper, &IperfWrapper::workFinished, m_thread, &QThread::quit);
    connect(m_iperfwrapper, &IperfWrapper::workFinished, m_iperfwrapper, &IperfWrapper::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

}

void IperfFileWorker::start()
{
    if (m_thread){
        m_thread->start();
    }
}

void IperfFileWorker::onProgress(int currentlineno)
{
    emit progress(currentlineno);
}

void IperfFileWorker::onThroughputData(int midx, QString sInterval, QString data)
{
    if(m_bidirtag.isEmpty()){
        qDebug() << "No m_bidirtag, not reprot ThroughputData: ("<<sInterval<<")" << data;
    }else{
        //collect all data value

        QJsonParseError error;
        //data:  "[{\"dir\":\"Rx\",\"idx\":\"5c\",\"jitter\":\"0.094\",\"jitter_unit\":\"ms\",\"packet_lost\":\"0\",\"packet_total\":\"8564\",\"unit\":\"Mbits/sec\",\"value\":\"25.0\"}]"
        QJsonDocument doc=QJsonDocument::fromJson(data.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonArray jArr = doc.array();
            // qDebug() << "IperfFileWorker::onThroughputData: " << sInterval << " : "<< data;
            double fInterval = sInterval.toDouble();
            for (QJsonArray::const_iterator it=jArr.constBegin(); it!=jArr.constEnd(); ++it) {
                QJsonObject jObj= it->toObject();
                bool avg= jObj.value("AVG").toBool();
                QString dir= jObj.value("dir").toString();
                QString idx = QString::number(midx) + "_"+ jObj.value("idx").toString();
                double tpvalue = jObj.value("value").toString().toDouble(); // jsondata value is string, need toString() then can convert to double!!
                QString unit = jObj.value("unit").toString();
                double pkt_lost = jObj.value("packet_lost").toString().toDouble(); // jsondata value is string, need toString() then can convert to int!!
                int pkt_total = jObj.value("packet_total").toString().toInt(); // jsondata value is string, need toString() then can convert to int!!
                QString jitter = jObj.value("jitter").toString();                   //TODO jitter
                QString jitter_unit = jObj.value("jitter_unit").toString();         //TODO jitter_unit
                double lostrate=0.0;
                if (pkt_total>0){
                    lostrate = (pkt_lost/pkt_total)*100;
                    // qDebug() << sInterval << " pkt_lost" << pkt_lost << " pkt_total:" << pkt_total << " ==" << lostrate;
                    if (lostrate>0){
                        qDebug() << "sInterval: " << sInterval << " lostrate" << lostrate;
                    }
                }
                Q_UNUSED(jitter)
                Q_UNUSED(jitter_unit)
                if (avg){
                    // AVG value
                    //QString midx, QString sInterval, QString idx, QString value, QString unit, QString dir, QString pkt_lost, QString pkt_total
                    emit updateTPAvg(QString::number(midx), sInterval, jObj.value("idx").toString(), QString::number(tpvalue),
                                     unit, dir, QString::number(pkt_lost), QString::number(pkt_total));
                } else {
                    TPData *tpdata = new TPData();
                    if ((m_datas.keys().length() > 0) && (m_datas.keys().contains(idx))){
                        tpdata = m_datas.value(idx);
                    }else {
                        m_datas.insert(idx, tpdata);
                    }
                    tpdata->timeDatas.append(fInterval);

                    tpdata->valueDatas.append(tpvalue);
                    tpdata->packetLost.append(pkt_lost);
                    tpdata->packetTotal.append(pkt_total);
                    tpdata->lostrate.append(lostrate);
                }
            }
        }
        else {
            qDebug() << "TPMgr::onIperfTPdata wrong format:(" << error.errorString() << "\n" << data;
            return;
        }
    }
}

void IperfFileWorker::onWorkFinished()
{
   foreach(QString idx, m_datas.keys()){
       emit updateTPDatas(idx, m_datas.value(idx)->timeDatas, m_datas.value(idx)->valueDatas,
                          m_datas.value(idx)->packetLost, m_datas.value(idx)->packetTotal, m_datas.value(idx)->lostrate);
   }
}
