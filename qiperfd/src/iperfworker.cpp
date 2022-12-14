#include "iperfworker.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <signal.h>
#include <QFileInfo>
//#include <QOverload>

#include <QDebug>

IperfWorker::IperfWorker(int idx, int version, QString cmd, QString arg, uint port, QObject *parent)
    : QObject{parent}
{
    m_idx = idx;
//    this->deleteLater(); //this will cause stdout not flush??
    m_parent = parent;
    m_version = version;
//    emit log(QString("arg:"+arg));
    m_cmd = cmd;
    m_arguments = arg.split(" ");
    m_port = port;
    m_arguments.append("-p");
    m_arguments.append(QString::number(m_port));
    if (version>=3){
        m_arguments.append("--forceflush");
    }
}

IperfWorker::~IperfWorker()
{
    if (!m_iperf->atEnd()){
        emit log(m_idx, "force kill iperf procress");
        m_iperf->kill();
    }
}

void IperfWorker::work()
{
    //this code run in thread
    m_stop = false;

    //create iperf procress
    m_iperf =  new QProcess(m_parent);
    m_iperf->start();
    connect(m_iperf, &QProcess::readyReadStandardOutput, this, &IperfWorker::readyReadStdOut);
    connect(m_iperf, &QProcess::readyReadStandardError, this, &IperfWorker::readyReadStdErr);
    connect(m_iperf, SIGNAL(readyRead()), this, SLOT(readyReadStdOut()));
    connect(m_iperf, SIGNAL(started()), this, SLOT(onStarted()));
//    connect(m_iperf, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(m_iperf, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IperfWorker::onFinished);
    //errorOccurred(QProcess::ProcessError error)

    m_iperf->start(m_cmd, m_arguments, QProcess::Unbuffered | QProcess::ReadWrite);
    if (m_iperf->waitForStarted()){
        emit log(m_idx, "start iperf (pid:"+ QString::number(m_iperf->processId())+")");
        emit log(m_idx, "iperf: " + m_cmd + " "+  m_arguments.join(" "));
        while (!m_stop){
            //procress iperf output
            QThread::msleep(500);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }else{
        emit log(m_idx, "iperf not started!!" + m_cmd + " " + m_arguments.join(" "));
        emit log(m_idx, m_iperf->readAllStandardError());
    }
//    emit finished(m_iperf->exitCode(), m_iperf->exitStatus());
}
void IperfWorker::setStop()
{
    m_stop = true;
    if (m_iperf->waitForFinished(1000)){
        emit log(m_idx, "iperf killed");
    }else{
//        emit log("kill iperf ");
        m_iperf->terminate();

    }
//    emit log("setStop");
    emit finished(m_idx, 0, QProcess::NormalExit);
}

void IperfWorker::onStarted()
{
    emit started(m_idx);
}

void IperfWorker::readyReadStdOut()
{
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardOutput();

    if (processOutput.length()>0){
        qDebug() << "Output was " << QString(processOutput);
        emit onStdout(m_idx, QString(processOutput));
    }
}

void IperfWorker::readyReadStdErr()
{
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardError();

    qDebug() << "Error was " << QString(processOutput);

    emit onStderr(m_idx, QString(processOutput));

}

void IperfWorker::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(m_idx, exitCode, int(exitStatus));
    m_stop = true;
}
