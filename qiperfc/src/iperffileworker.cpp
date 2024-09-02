#include "iperffileworker.h"

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

    connect(m_thread, &QThread::started, m_iperfwrapper, &IperfWrapper::work);
//    connect(m_thread, &QThread::finished, m_iperfwrapper, &IperfWrapper::onFinished);
    m_iperfwrapper->moveToThread(m_thread);
    QObject::connect(m_iperfwrapper, &IperfWrapper::workFinished, m_thread, &QThread::quit);
    QObject::connect(m_iperfwrapper, &IperfWrapper::workFinished, m_iperfwrapper, &IperfWrapper::deleteLater);
    QObject::connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);

}

void IperfFileWorker::start()
{
    if (m_thread){
        m_thread->start();
    }
}

void IperfFileWorker::onThroughputData(int idx, QString sInterval, QString data)
{
    if(m_bidirtag.isEmpty()){
        qDebug() << "No m_bidirtag, not reprot ThroughputData: ("<<sInterval<<")" << data;
    }else{
//        qDebug() << "sInterval:" << sInterval;
        //DEBUG: this will not get in line order => cause data show on UI not final data!!
        emit onThroughput(idx, sInterval, data);
    }
}
