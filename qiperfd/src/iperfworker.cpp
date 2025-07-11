#include "iperfworker.h"
#include "iperfworker.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QThread>
#include <signal.h>
#include <QFileInfo>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDir>
#include <QDateTime>
// Platform-specific includes for native thread IDs
#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <pthread.h>
#endif

#include "../src/tpmgrdata.h"
// #include <QOverload>

#include "../src/comm.h"

IperfWorker::IperfWorker(qint64 idx, int version, QString cmd, QString arg,
                         uint port, QString bindaddr, QString target,
                         bool bidir, bool reverse, int interval,
                         int delaystart, bool ignoreWrongInterval,
                         bool restartonerror, QJsonObject restartrule,
                         QObject *parent)
    : QObject{parent}, m_idx(idx), m_version(version), m_cmd(cmd), m_port(port),
      m_bindaddr(bindaddr), m_target(target), m_bidir(bidir), m_reverse(reverse),
    m_interval(interval), m_delaystart(delaystart),
    m_ignoreWrongInterval(ignoreWrongInterval),
    m_restartonerror(restartonerror), m_restartrule(restartrule),
    m_parent(parent)
{
    m_debuglv = 3;
    m_logfile = nullptr;
    m_logtextstream = nullptr;
    m_restartonErrorStop = false;
    m_restartonNormalStop = false;
    m_iperflogpath = "";
    if (m_restartonerror){
        m_restartonErrorStop = m_restartrule["restartonErrorStop"].toBool();
        m_restartonNormalStop = m_restartrule["restartonNormalStop"].toBool();
        m_restartrules = m_restartrule["iperfRestartRules"].toArray();
    }
    m_iperfwrapper = new IperfWrapper(m_ignoreWrongInterval);
    m_iperfwrapper->setDelaytime(delaystart);
    m_iperfwrapper->setInterval(interval);
    m_iperfwrapper->setArgs(arg);
    connect(m_iperfwrapper, &IperfWrapper::sendThroughput, this, &IperfWorker::onThroughputData);
    connect(m_iperfwrapper, &IperfWrapper::debuginfo, this, &IperfWorker::onDebuginfo);
//    this->deleteLater(); //this will cause stdout not flush??
//    m_cmd = cmd; //iperf exec fullpath
#if QT_VERSION < 0x050E00 // < 5.14.0
    m_arguments = arg.split(" ", QString::SkipEmptyParts);
#else
    m_arguments = arg.split(" ", Qt::SkipEmptyParts);
#endif
    if (m_arguments.contains("-s")){
        m_servermode=true;
    }
    int omitidx = m_arguments.indexOf("--omit");
    m_omit = m_arguments.value(omitidx+1, 0).toInt();
    int durationidx = m_arguments.indexOf("-t");
    m_duration = m_arguments.value(durationidx+1, 0).toInt();

//    m_port = port;
//    m_bindaddr = bindaddr;
//    m_target = target;
    if (m_servermode){
        if (m_bidir){
            setBidirTag(TPDIRRx);
        }else if(m_reverse){
            setBidirTag(TPDIRNO);
        }else{
            setBidirTag(TPDIRRx);
        }
    }else{
        if (m_bidir){
            setBidirTag(TPDIRTx);
        }else if (m_reverse){
            setBidirTag(TPDIRTx);
        }else{
            setBidirTag(TPDIRNO);
        }
    }
    // m_interval = interval;
    // debug("init[" + getBindKey() + "] reg m_bidirtag:" + m_bidirtag + " interval:" + QString::number(m_interval));
    m_selfdestructionTime = (10+m_interval+m_delaystart) * 1000; //10 sec + report interval
    m_selfdestruction = new QTimer(this);
    m_selfdestruction->setInterval(m_selfdestructionTime);
    connect(m_selfdestruction, &QTimer::timeout, this, &IperfWorker::onSelfDestructor);
    connect(this, &IperfWorker::stopSelfDestructor, m_selfdestruction, &QTimer::stop);

}

IperfWorker::~IperfWorker()
{
    if (!m_iperf->atEnd()){
        // emit log(m_idx, "force kill iperf procress");
        debug("force kill iperf procress");
        m_iperf->kill();
    }
}

void IperfWorker::work()
{   //this code run in another thread
    m_threadid = getThreadID();

    debug(QString("restartonerror:%1").arg(m_restartonerror?"true":"false"));
    debug(QString("restartonErrorStop:%1").arg(m_restartonErrorStop?"true":"false"));
    debug(QString("restartonNormalStop:%1").arg(m_restartonNormalStop?"true":"false"));

    QJsonDocument doc(m_restartrules);
    // m_restartrule.to;
    // 1. Convert QVariantMap to QJsonObject
    // QJsonObject jsonObject = QJsonObject::fromVariantMap(m_restartrule);
    // // // 2. Create a QJsonDocument from the QJsonObject
    // QJsonDocument doc(jsonObject);
    // // // 3. Convert the QJsonDocument to a QString
    // // //    QJsonDocument::toJson() returns a QByteArray, so convert it to QString.
    // // //    QJsonDocument::Indented for pretty-printing, QJsonDocument::Compact for minimal.
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    debug("restartrule: "+jsonString, 3);


    try{
        m_stop = false;
        //create iperf procress
        m_iperf =  new QProcess(m_parent);
        m_iperf->setProgram(m_cmd);
        debug("IperfWorker::work: " + m_cmd + " args:" + m_arguments.join(" "));
        m_iperf->setArguments(m_arguments);
        connect(m_iperf, &QProcess::readyReadStandardOutput, this, &IperfWorker::readyReadStdOut);
        connect(m_iperf, &QProcess::readyReadStandardError, this, &IperfWorker::readyReadStdErr);
    //    connect(m_iperf, &QProcess::readyRead, this, &IperfWorker::readyReadStdOut);
        connect(m_iperf, &QProcess::started, this, &IperfWorker::onStarted);
        connect(m_iperf, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &IperfWorker::onFinished);

        if (m_delaystart>0){
            debug("m_delaystart: " + QString::number(m_delaystart));
            emit started(m_refrow, m_servermode, getBindKey());// TODO: good place to notice started??
            QDateTime waitStartTime = QDateTime::currentDateTime();
            QDateTime waitEndTime = QDateTime::currentDateTime();
            int iWait = waitStartTime.secsTo(waitEndTime);
            while (iWait < m_delaystart){
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                QThread::msleep(100);
                waitEndTime = QDateTime::currentDateTime();
                iWait = waitStartTime.secsTo(waitEndTime);
            }
        }

        m_iperf->start();
        if (m_iperf->waitForStarted()){
            if (m_selfdestruction->isActive()){
                emit stopSelfDestructor();
            }
            // emit log(m_idx, "start iperf (pid:"+ QString::number(m_iperf->processId())+")");
            QString msg = QString("start iperf %1(pid:%2)").arg(m_servermode?"server":"client",
                                                              QString::number(m_iperf->processId()));
            debug(msg, 3);
            // emit log(m_idx, "iperf: \"" + QDir::toNativeSeparators(m_cmd) + "\" "+  m_arguments.join(" "));
            debug("iperf: \"" + QDir::toNativeSeparators(m_cmd) + "\" "+  m_arguments.join(" "), 3);
            while (!m_stop){
                //procress iperf output
                QThread::msleep(500);
                QCoreApplication::processEvents(QEventLoop::AllEvents); // must have
            }
        }else{
            // emit log(m_idx, "iperf not started!! \"" + QDir::toNativeSeparators(m_cmd) + "\" " + m_arguments.join(" "));
            debug("iperf not started!! \"" + QDir::toNativeSeparators(m_cmd) + "\" " + m_arguments.join(" "), 4);
            // emit log(m_idx, m_iperf->readAllStandardError());
            debug(m_iperf->readAllStandardError(), 4);
        }
    }catch (const std::exception &e) {
        // Handle the exception and show an error message
        debug("IperfWorker::work Exception Caught" + QString(e.what()));
    }catch (...){
        debug("IperfWorker::work Unknown ERROR");
    }
}

bool IperfWorker::isRunning()
{
    return m_running;
}

void IperfWorker::onSelfDestructor()
{
    debug("onSelfDestructor");
    m_selfdestruction->stop();
    setStop();
}

void IperfWorker::onSetDebugLv(int lv)
{

    m_debuglv = lv;
}

void IperfWorker::setStop()
{
    m_stop = true;
    if (m_iperf && m_iperf->state() == QProcess::Running) {
        m_iperf->terminate(); // Attempt graceful termination
        if (m_iperf->waitForFinished(3000)){
            // emit log(m_idx, "iperf killed");
            debug("iperf killed", 4);
        }else{
            if (m_iperf->state() == QProcess::Running) {
                int pid = m_iperf->processId();
                if (pid >0){
                    debug("force terminate iperf id: " + QString::number(pid));
        #if defined(Q_OS_WIN32)
                    m_iperf->kill();
        #else
                    m_iperf->terminate();
        #endif
                }else{
                    debug("NOT Running m_iperf: " + m_iperf->program() + m_iperf->arguments().join(" "));
                }
            }
        }
    }
    emit workerFinished(m_idx, m_servermode); // Notify manager that this worker is logically done
    this->deleteLater(); // Schedule deletion on this thread's event loop
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

void IperfWorker::setExtra(QString parallel, QString protocal, uint port)
{
    m_parallel=parallel;
    m_protocal=protocal;
    m_port = port;
    if (m_iperfwrapper){
        m_iperfwrapper->setIperf(QString::number(m_version), m_protocal, m_port);
    }
}

void IperfWorker::toLogFile(QString msg)
{
    if(m_logtextstream!=nullptr){
        *m_logtextstream << msg;
        m_logtextstream->flush();
    }else{
        debug("toLogFile: m_logtextstream not exist");
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

void IperfWorker::debug(QString msg, int debuglv)
{
    if (debuglv<=m_debuglv){
        emit debuginfo("("+m_threadid+")"+"-"+QString::number(m_idx)+"-"+msg);
    }
}

QString IperfWorker::getThreadID()
{
    QString id="N/A";
#ifdef Q_OS_WIN
    DWORD nativeThreadId = GetCurrentThreadId();
    id = QString::number(nativeThreadId);
#else
    #ifdef Q_OS_LINUX
        pthread_t nativeThreadId = pthread_self();
        // 1. Convert pthread_t (long unsigned int) to std::string
        //    std::to_string has overloads for various integer types, including unsigned long.
        std::string std_str_pth_id = std::to_string(nativeThreadId);
        // 2. Convert std::string to QString
        id = QString::fromStdString(std_str_pth_id);
    #else
        debug("Not support platform to get real thread id", 10);
    #endif
#endif
    return id;
}

void IperfWorker::onStarted()
{
    // TODO: restart should not use new log file?
    QString tmp = m_iperflogpath+QDir::separator()+getBindKey()+".log";
    m_logfile=new QFile(tmp);
    debug("m_logfile: " + QDir::toNativeSeparators(m_logfile->fileName()));

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
    // add iperf args
    toLogFile(QString("### %1 %2 \n").arg(m_cmd, m_arguments.join(" ")));
    // add start time
    QString stime = QDateTime::currentDateTime().toString(DATETIME_NOW_FORMAT);
    toLogFile(QString("### %1 \n").arg(stime));
}

void IperfWorker::readyReadStdOut()
{
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardOutput();

    if (processOutput.length()>0){
        toLogFile(processOutput);
        foreach (auto line , QString(processOutput).split("\n")){
            //ignore empty line
            if (line.length()>0){
                parserStdOut(line);
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }

    }
}

void IperfWorker::readyReadStdErr()
{
    QByteArray processOutput;
    processOutput = m_iperf->readAllStandardError();
    QString err = QString(processOutput);
    if (processOutput.length()>0){
        debug("readyReadStdErr: " + m_arguments.join(" ") +"\n"+ err);
        toLogFile(err);
    }
    if (m_version==2){
        if (err.contains("WARNING")){
            // ignore iperf2 WARNING, eq:
            // -t 0 => run forever
//WARNING: client will send traffic forever or until an external signal (e.g. SIGINT or SIGTERM) occurs to stop it
            return;
        }
    }
    m_running = false;
    m_stop = true;
    emit onStderr(m_idx, m_refrow, err, getBindKey());
    //When there is stderr output, this will cause TP test stop
    // onFinished(1, QProcess::ExitStatus(1)); // something error

}

void IperfWorker::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitCode==0){
        //normal stop
        if (m_restartonNormalStop){
            debug("TODO: restartonNormalStop", 3);
            if (!m_servermode){
                //client mode have duration, info qiperf console to extend wait time
                emit iperfExtendWait(m_idx, m_duration);
            }
            // TODO setup restart?
            return;
        }
    }else{
        //error stop
        if (m_restartonErrorStop) {
            debug("TODO: restartonErrorStop", 3);
            if (!m_servermode){
                //client mode have duration, info qiperf console to extend wait time
                emit iperfExtendWait(m_idx, m_duration);
            }
            // TODO setup restart?
            return;
        }
    }
    if (m_selfdestruction->isActive()){
        debug("onFinished: stop m_selfdestructor");
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
    emit finished(m_refrow, exitCode, int(exitStatus), getBindKey(), filename, m_servermode);
}

void IperfWorker::parserStdOut(QString msg)
{
    if (m_version==3){
        m_iperfwrapper->parserIperf3(msg);
    }else if (m_version==2){
        m_iperfwrapper->parserIperf2(msg);
    }else {
        debug("parserStdOut: Not support iperf version:" +QString::number(m_version));
    }
}

void IperfWorker::onThroughputData(int refrow, QString sInterval, QString data)
{
    if(m_bidirtag.isEmpty()){
        debug("No m_bidirtag, not reprort ThroughputData: ("+sInterval+")" + data, 6);
    }else{
        debug("[IperfWorker::onThroughputData]refrow:" + QString::number(refrow) + " sInterval:" + sInterval, 5);
        emit iperfTPdata(refrow, sInterval, data);
    }
}

void IperfWorker::onDebuginfo(QString msg)
{
    debug("IperfWorker::onDebuginfo:" + msg, 4);
    emit debuginfo("[IperfWrapper]" + msg);
}
