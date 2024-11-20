#include "iperfworker.h"
#include "iperfworker.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <signal.h>
#include <QFileInfo>
#include <QJsonValue>
#include <QJsonObject>
#include <QDir>
#include <QDateTime>

//#include <QOverload>

#include <QDebug>
#include "../src/comm.h"

IperfWorker::IperfWorker(int idx, int version, QString cmd, QString arg,
                         uint port, QString bindaddr, QString target,
                         bool bidir, bool reverse, int interval,
                         int delaystart,
                         QObject *parent)
    : QObject{parent}, m_idx(idx), m_version(version), m_cmd(cmd), m_port(port),
      m_bindaddr(bindaddr), m_target(target), m_bidir(bidir), m_reverse(reverse),
      m_interval(interval), m_delaystart(delaystart), m_parent(parent)
{
//    m_delaystart =delaystart; //TODO: m_delaystart
    m_logfile = nullptr;
    m_logtextstream = nullptr;
    m_iperflogpath = "";
//    m_idx = idx; // thread index
    m_iperfwrapper = new IperfWrapper(this);
    m_iperfwrapper->setDelaytime(delaystart);
    connect(m_iperfwrapper, &IperfWrapper::sendThroughput, this, &IperfWorker::onThroughputData);
//    this->deleteLater(); //this will cause stdout not flush??
//    m_parent = parent;
//    m_version = version;
//    m_bidir = bidir;
//    m_reverse = reverse;
//    emit log(QString("arg:"+arg));
//    m_cmd = cmd; //iperf exec fullpath
    m_arguments = arg.split(" ");
    if (m_arguments.contains("-s")){
        m_servermode=true;
        if (m_version==3){
            m_arguments.append("--one-off"); //handle one client connection then exit
        }
    }
//    m_port = port;
//    m_bindaddr = bindaddr;
//    m_target = target;
    if (m_version>=static_cast<int>(IPERF_VER::V3)){
        m_arguments.append("--forceflush");
    }
    if (m_servermode){
        if (m_reverse){
            if (m_bidir){
                setBidirTag("Rx");
            }else{
                setBidirTag("");
            }
        }else{
            setBidirTag("Tx");
        }
    }else{
        if (m_reverse||m_bidir){
            setBidirTag("Rx");
        }else{
            if (m_bidir){
                setBidirTag("Tx");
            }else{
                setBidirTag("");
            }
        }
    }
//    m_interval = interval;
    qDebug() << "init[" << getBindKey() << "] reg m_bidirtag:" << m_bidirtag << " interval:" << m_interval;
    m_selfdestructionTime = (10+m_interval+m_delaystart) * 1000; //10 sec + report interval
    m_selfdestruction = new QTimer(this);
    m_selfdestruction->setInterval(m_selfdestructionTime);
    connect(m_selfdestruction, &QTimer::timeout, this, &IperfWorker::onSelfDestructor);
    connect(this, &IperfWorker::stopSelfDestructor, m_selfdestruction, &QTimer::stop);

}

IperfWorker::~IperfWorker()
{
    if (!m_iperf->atEnd()){
        emit log(m_idx, "force kill iperf procress");
        m_iperf->kill();
    }
}

void IperfWorker::work()
{   //this code run in another thread
    try{
        m_stop = false;
        //create iperf procress
        m_iperf =  new QProcess(m_parent);
        m_iperf->setProgram(m_cmd);
        m_iperf->setArguments(m_arguments);
        connect(m_iperf, &QProcess::readyReadStandardOutput, this, &IperfWorker::readyReadStdOut);
        connect(m_iperf, &QProcess::readyReadStandardError, this, &IperfWorker::readyReadStdErr);
    //    connect(m_iperf, &QProcess::readyRead, this, &IperfWorker::readyReadStdOut);
        connect(m_iperf, &QProcess::started, this, &IperfWorker::onStarted);
        connect(m_iperf, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IperfWorker::onFinished);

        qDebug() << "m_delaystart: " << QString::number(m_delaystart);
        if (m_delaystart>0){
            emit started(m_refrow, m_servermode, getBindKey());// TODO: good place to notice started??
            QDateTime waitStartTime = QDateTime::currentDateTime();
            QDateTime waitEndTime = QDateTime::currentDateTime();
            int iWait = waitStartTime.secsTo(waitEndTime);
            while (iWait < m_delaystart){
                // qDebug() << "wait " << QString::number(m_delaystart-iWait) << " to start iperf";
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                QThread::msleep(100);
                waitEndTime = QDateTime::currentDateTime();
                iWait = waitStartTime.secsTo(waitEndTime);
            }
        }

        m_iperf->start();
        if (m_iperf->waitForStarted()){
            emit log(m_idx, "start iperf (pid:"+ QString::number(m_iperf->processId())+")");
            emit log(m_idx, "iperf: \"" + QDir::toNativeSeparators(m_cmd) + "\" "+  m_arguments.join(" "));
            while (!m_stop){
                //procress iperf output
                QThread::msleep(500);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
        }else{
            emit log(m_idx, "iperf not started!! \"" + QDir::toNativeSeparators(m_cmd) + "\" " + m_arguments.join(" "));
            emit log(m_idx, m_iperf->readAllStandardError());
        }
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        qDebug() << "IperfWorker::work Exception Caught" << e.what();
    }catch (...){
        qDebug() << "IperfWorker::work Unknown ERROR";
    }
}

bool IperfWorker::isRunning()
{
    return m_running;
}

void IperfWorker::onSelfDestructor()
{
    qDebug() << "onSelfDestructor";
    m_selfdestruction->stop();
    setStop();
}

void IperfWorker::setStop()
{
    m_stop = true;
    if (m_iperf->waitForFinished(1000)){
        emit log(m_idx, "iperf killed");
    }else{
        int pid = m_iperf->processId();
        if (pid >0){
            qDebug() << "force terminate iperf id: " << QString::number(pid);
            m_iperf->terminate();
        }else{
            qDebug() <<"NOT Running m_iperf: " << m_iperf->program() << m_iperf->arguments();
        }
    }
//    emit finished(m_refrow, 0, 2, getBindKey());
}

QString IperfWorker::getBindKey()
{
    if (m_servermode){
        return m_bindaddr + "_" + QString::number(m_port);
    }else{
        return m_bindaddr + "-" + m_target + "_" + QString::number(m_port);
    }
}

void IperfWorker::setIperfLogPath(QString filepath)
{
    m_iperflogpath = filepath;
}

void IperfWorker::setBidirTag(QString bidir)
{
    m_bidirtag = bidir;
}

void IperfWorker::setRefRow(QString refrow)
{
    m_refrow = refrow.toInt();
}

void IperfWorker::setExtra(QString parallel, QString protocal)
{
    m_parallel=parallel;
    m_protocal=protocal;
    if (m_iperfwrapper){
        m_iperfwrapper->setIperf(QString::number(m_version), m_protocal);
    }
}

void IperfWorker::toLogFile(QString msg)
{
    if(m_logtextstream!=nullptr){
        *m_logtextstream << msg;
        m_logtextstream->flush();
    }else{
        qDebug() << "toLogFile: m_logtextstream not exist";
    }
}

bool IperfWorker::getServerMode()
{
    return m_servermode;
}

int IperfWorker::getRefRow()
{
    return m_refrow;
}

void IperfWorker::onStarted()
{
    QString tmp = m_iperflogpath+QDir::separator()+getBindKey()+".log";
    m_logfile=new QFile(tmp);
    qInfo() << "m_logfile: " << QDir::toNativeSeparators(m_logfile->fileName());

    if(m_logfile->open(QIODevice::WriteOnly|QIODevice::Append)){
        m_logtextstream = new QTextStream(m_logfile);
    }else{
        emit onStderr(m_idx, m_refrow, "ERROR: open file '"+ tmp +"' Fail", getBindKey());
    }
    m_running = true;
    m_iperfwrapper->setSetting(m_refrow, m_servermode, m_parallel, m_bidir, m_bidirtag);
    m_selfdestruction->start();
    if (m_delaystart==0){
        emit started(m_refrow, m_servermode, getBindKey());// TODO: good place to notice started??
    }
}

void IperfWorker::readyReadStdOut()
{
    if (m_selfdestruction->isActive()){
        // qInfo() << "readyReadStdOut: stop m_selfdestructor";
        emit stopSelfDestructor();
    }
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardOutput();

    if (processOutput.length()>0){
        toLogFile(processOutput);
        foreach (auto line , QString(processOutput).split("\n")){
            //ignore empty line
            if (line.length()>0){
                parserStdOut(line);
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }

    }
}

void IperfWorker::readyReadStdErr()
{
    if (m_selfdestruction->isActive()){
        qInfo() << "readyReadStdErr: stop m_selfdestructor";
        emit stopSelfDestructor();
    }
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardError();
    QString err = QString(processOutput);
    if (processOutput.length()>0){
        qDebug() << "readyReadStdErr: " << err;
        toLogFile(processOutput);
    }

    m_running = false;
    m_stop = true;
    emit onStderr(m_idx, m_refrow, err, getBindKey());
    onFinished(1, QProcess::ExitStatus(1)); // something error

}

void IperfWorker::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_selfdestruction->isActive()){
        qInfo() << "onFinished: stop m_selfdestructor";
        emit stopSelfDestructor();
    }
    QString filename="";
    if(m_logfile !=nullptr){
        m_logfile->flush();
        m_logfile->close();
        filename = m_logfile->fileName();
        delete m_logfile;
        m_logfile = nullptr;
    }
    if(m_logtextstream !=nullptr){
        delete m_logtextstream;
        m_logtextstream = nullptr;
    }

    m_running = false;
    m_stop = true;
    emit finished(m_refrow, exitCode, int(exitStatus), getBindKey(), filename);
}

void IperfWorker::parserStdOut(QString msg)
{
    if (m_version==3){
        m_iperfwrapper->parserIperf3(msg);
    }else if (m_version==2){
        qDebug() << "TODO: parser iperf2 output";
    }else {
        qDebug() << "parserStdOut: Not support iperf version:" <<m_version;
    }
}

void IperfWorker::onThroughputData(int idx, QString sInterval, QString data)
{
    if(m_bidirtag.isEmpty()){
//        qDebug() << "No m_bidirtag, not reprot ThroughputData: ("<<sInterval<<")" << data;
    }else{
        emit onThroughput(idx, sInterval, data);
    }
}
