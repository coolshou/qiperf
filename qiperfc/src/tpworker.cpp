#include "tpworker.h"

#include "comm.h"
#include <QDir>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>
#include <QMutexLocker>

#include "myfunc.h"
#include "tpstatus.h"

TpWorker::TpWorker(QString logpath, QList<TP *> &tps, bool ignoreWrongInterval,
                   int wstimeout,
                   int extrawait, int waitserverready,
                   QObject *parent)
    : QObject{parent}, m_logpath(logpath), m_tps(tps),
    m_ignoreWrongInterval(ignoreWrongInterval),
    iWSTimeout(wstimeout),
    iExtraWait(extrawait), m_WaitServerReady(waitserverready)
{
    m_debuglv = 3;
    m_extendwaittime = 0;
    m_flowManager= new TpFlowManager(this);
    connect(m_flowManager, &TpFlowManager::debugMeg, this, &TpWorker::debuginfo);
    connect(m_flowManager, &TpFlowManager::StatusReady, this, &TpWorker::onStatusReady);
    connect(m_flowManager, &TpFlowManager::WaitTimeout, this, &TpWorker::onWaitTimeout);

    // m_statusChecker= new TPStatusChecker(this);
    // // Connect m_statusChecker's signals to TpWorker's slots
    // connect(m_statusChecker, &TPStatusChecker::statusAchieved, this, &TpWorker::handleStatusAchieved);
    // connect(m_statusChecker, &TPStatusChecker::timeoutOccurred, this, &TpWorker::handleTimeoutOccurred);
    // connect(m_statusChecker, &TPStatusChecker::monitoringFinished, this, &TpWorker::handleMonitoringFinished);
    // //Connect KSC's request for status to HLW's slot that provides it
    // connect(m_statusChecker, &TPStatusChecker::requestCurrentStatus, this, &TpWorker::provideCurrentStatus);

}

TpWorker::~TpWorker()
{

}

bool TpWorker::waitAllServerReady()
{
    //wait all m_status_server enter TPStatus::started
    QDateTime sTime = QDateTime::currentDateTime();
    bool bServerReady=false;
    // int waitedSecs = 0;

    while (!bServerReady && !bUserStop) {
        // this is a blocking loop, not good
        QThread::msleep(100);  // check 5 times per second
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        int readyCount = 0;
        for (const auto &skey : m_status_server.keys()) {
            if (m_status_server.value(skey) == TPStatus::started) {
                readyCount++;
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents);

        }

        if (readyCount >= m_status_server.size()) {
            bServerReady = true;
            break;  // all servers ready!
        }

        qint64 elapsed = sTime.secsTo(QDateTime::currentDateTime());
        qint64 remaining = m_WaitServerReady - elapsed;

        emit updateStatus(QString("Wait server ready: %1/%2 (%3s left)")
                              .arg(readyCount)
                              .arg(m_status_server.size())
                              .arg(remaining));

        if (remaining <= 0) {
            debug("[TpWorker]Server ready TIMEOUT",4);
            break;
        }
    }

    return bServerReady;
}

bool TpWorker::waitServerReady(QString skey, TPStatus::Status expectstatus)
{
    QDateTime sTime = QDateTime::currentDateTime();
    bool bReady=false;
    // int waitedSecs = 0;
    qint64 elapsed = sTime.secsTo(QDateTime::currentDateTime());
    qint64 remaining = m_WaitServerReady - elapsed;
    while (!bReady && !bUserStop) {
        // this is a blocking loop, not good
        QThread::msleep(200);  // check 5 times per second
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        debug(QString("(%1)m_status_server: %2").arg(QString::number(remaining),
                                                     QString::number(m_status_server.value(skey))));
        if (m_status_server.value(skey) == static_cast<int>(expectstatus)) {
            bReady=true;
            break;
        }
        elapsed = sTime.secsTo(QDateTime::currentDateTime());
        remaining = m_WaitServerReady - elapsed;
        if (remaining <= 0) {
            debug(QString("[TpWorker]Wait Server %1 ready TIMEOUT").arg(skey), 3);
            break;
        }
    }
    return bReady;
}

bool TpWorker::waitAllClientReady()
{
    //wait all m_status_client enter TPStatus::started
    QDateTime sTime = QDateTime::currentDateTime();
    bool bClientReady=false;
    // int waitedSecs = 0;

    while (!bClientReady && !bUserStop) {
        // this is a blocking loop, not good
        QThread::msleep(200);  // check 5 times per second
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        int readyCount = 0;
        for (const auto &skey : m_status_client.keys()) {
            if (m_status_client.value(skey) == TPStatus::started) {
                readyCount++;
            }
        }

        if (readyCount >= m_status_client.size()) {
            bClientReady = true;
            break;  // all Client ready!
        }

        qint64 elapsed = sTime.secsTo(QDateTime::currentDateTime());
        qint64 remaining = m_WaitServerReady - elapsed;

        emit updateStatus(QString("Wait Client ready: %1/%2 (%3s left)")
                              .arg(readyCount)
                              .arg(m_status_client.size())
                              .arg(remaining));

        if (remaining <= 0) {
            debug("[TpWorker]Client ready TIMEOUT",4);
            break;
        }
    }

    return bClientReady;
}

bool TpWorker::waitClientReady(QString skey, TPStatus::Status expectstatus)
{
    QDateTime sTime = QDateTime::currentDateTime();
    bool bReady=false;
    // int waitedSecs = 0;

    while (!bReady && !bUserStop) {
        // this is a blocking loop, not good
        QThread::msleep(200);  // check 5 times per second
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        if (m_status_client.value(skey) == static_cast<int>(expectstatus)) {
            bReady=true;
            break;
        }
        qint64 elapsed = sTime.secsTo(QDateTime::currentDateTime());
        qint64 remaining = m_WaitServerReady - elapsed;
        if (remaining <= 0) {
            debug(QString("[TpWorker]Wait Client %1 ready TIMEOUT").arg(skey), 3);
            break;
        }
    }
    return bReady;
}

void TpWorker::work()
{
    initStart();
    m_TestStartTime = QDateTime::currentDateTime();
    // m_throughputview->setStartTime(m_TestStartTime);
    QString startTime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
    m_datapath = m_logpath + QDir::separator() + startTime;
    QDir d(m_datapath);
    if (!d.exists()){
        d.mkpath(".");
    }
    QString err="";
    emit updateDatapath(m_datapath);
    emit updateStarttime(m_TestStartTime);
    emit updateRunStatus(true);
    //start test

    QString s; // websocket url
    QString cmd;
    qint64 rs=0;
    int maxtestduration=0; // max wait test time
    int maxInterval=0; // max Interval time
    int iwait=0;
    int iomit=0;
    int idelaytime=0;
    int itimeout;
    int refrow;
    bool isRunforever=false;
    int iSliceWindowTime=0;
    // list of throughput test pair
    foreach (TP *tp, m_tps) {
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
        if (tp->getEnabled()){
            tp->getParallel();

            if (tp->getInterval()> maxInterval){
                maxInterval = tp->getInterval();
            }
            //RPC to control all server endpoint (iperf server)
            isRunforever = isRunforever | tp->getRunforever();
            int testduration=0;
            iwait = tp->getWaitTime();
            if (iwait> testduration){
                testduration = iwait+iExtraWait;
            }
            iomit = tp->getOmitTime();
            testduration = testduration + iomit;
            idelaytime = tp->getDelaytime();
            if (idelaytime>0) {
                testduration = testduration + idelaytime;
            }
            if (testduration> maxtestduration){
                maxtestduration = testduration;
            }
            refrow = tp->row();
            // err = "=====[TpWorker]refrow:" + QString::number(refrow) + " maxtestduration:" + QString::number(maxtestduration);

            QString serverIP = tp->getMgrServer();
            //TODO: detect manager server is pingable
            err = "=====[TpWorker] server:" + serverIP;
            debug(err, 5);
            if (!m_wss.contains(serverIP)) {
                //TODO: can not work with interface with DHCP under Windows??
                s = "ws://"+serverIP+":"+QString::number(QIPERFD_WSPORT);
                err =  "[TpWorker]server websocket:" + serverIP + " url: " + s + " m_datapath:" + m_datapath;
                debug(err, 5);
                m_wss[serverIP]=new WSClient(serverIP, QUrl(s), m_datapath);
                connect(m_wss[serverIP], &WSClient::iperfStarted, this, &TpWorker::onIperfStarted);
                connect(m_wss[serverIP], &WSClient::iperfStoped, this, &TpWorker::onIperfStoped);
                connect(m_wss[serverIP], &WSClient::iperfReStarted, this, &TpWorker::onIperfReStarted);
                // connect(m_wss[serverIP], &WSClient::disconnected, this, &TpWorker::onServerDisconnected);
                connect(m_wss[serverIP], &WSClient::disconnected, this, &TpWorker::onDisconnected);
                connect(m_wss[serverIP], &WSClient::iperfTPdata, this, &TpWorker::onIperfTPdata);
                connect(m_wss[serverIP], &WSClient::iperfExtendWait, this, &TpWorker::onIperfExtendWait);
                connect(m_wss[serverIP], &WSClient::debuginfo, this, &TpWorker::onDebuginfo);
            }else{
                err =  "[TpWorker]m_wss exist:serverIP:" + serverIP;
                debug(err, 5);
                m_wss[serverIP]->setDatapath(m_datapath);
            }
            itimeout = iWSTimeout;
            while (itimeout>0 && (bErrorStop==0)&& (bUserStop==false)){
                QThread::msleep(1000);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                if (m_wss.contains(serverIP)){
                    if (m_wss.value(serverIP)->isConnected()){
                        break;
                    }
                }else{
                    bErrorStop = 1;
                    err = "ERROR: websocket "+serverIP+" not connected";
                    tp->setComment(err);
                    emit errorStop(2, err);
                    break;
                }
                itimeout--;
                emit updateStatus(" wait WSClient connect to server: "+ s +
                                  " ("+ QString::number(itimeout) +")");
            }
            if (itimeout<=0){
                emit errorStop(1,"ERROR: Wait connect to websocket " +s+ " timeout");
                break;
            }
            if(bErrorStop>0){
                emit errorStop(3, "Unknown error happen!!("+QString::number(bErrorStop)+")");
                return;
            }
            if(bUserStop){
                err = "[TpWorker]User Stop on wait server websocket connected!!";
                debug(err);
                return;
            }
            //tell server add iperf server
            //CMD_IPERF_ADD:<num>:<ignore>:<iperf args>
            cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow);
            cmd = cmd + ":"+ (m_ignoreWrongInterval?"1":"0");
            cmd = cmd + ":"+ tp->getServerArgs();
            debug("[TpWorker]server cmd:" + serverIP + " => " + cmd, 4);
            rs = m_wss[serverIP]->sendText(cmd);
            if (rs<=0){
                emit errorStop(1, "Setup server iperf config fail: "+ tp->getServerArgs());
                break;
            }
            QString skey = tp->getBindKey(true);
            debug("[TpWorker]m_status_server skey:" + skey);
            m_status_server[skey]=TPStatus::init; // init server of BindKey status 0
            //###### client ######

            //RPC to control all client endpoint (iperf client)
            QString clientIP = tp->getMgrClient();
            err = "=====[TpWorker] client:" + clientIP;
            debug(err, 5);
            //TODO: detect manager client is pingable
            if (!m_wsc.contains(clientIP)) {
                s = "ws://"+clientIP+":"+QString::number(QIPERFD_WSPORT);
                debug("[TpWorker]client websocket:" + clientIP + " url: " + s + " m_datapath:" + m_datapath);
                m_wsc[clientIP]=new WSClient(clientIP, QUrl(s), m_datapath);
                connect(m_wsc[clientIP], &WSClient::iperfStarted, this, &TpWorker::onIperfStarted);
                connect(m_wsc[clientIP], &WSClient::iperfStoped, this, &TpWorker::onIperfStoped);
                connect(m_wsc[clientIP], &WSClient::iperfReStarted, this, &TpWorker::onIperfReStarted);
                // connect(m_wsc[clientIP], &WSClient::disconnected, this, &TpWorker::onClientDisconnected);
                connect(m_wsc[clientIP], &WSClient::disconnected, this, &TpWorker::onDisconnected);
                connect(m_wsc[clientIP], &WSClient::iperfTPdata, this, &TpWorker::onIperfTPdata);
                connect(m_wsc[clientIP], &WSClient::iperfExtendWait, this, &TpWorker::onIperfExtendWait);
                connect(m_wsc[clientIP], &WSClient::debuginfo, this, &TpWorker::onDebuginfo);
            }else{
                err =  "[TpWorker]m_wsc exist:clientIP:" + clientIP;
                debug(err, 5);
                m_wsc[clientIP]->setDatapath(m_datapath);
            }
            itimeout = iWSTimeout;
            while (itimeout>0 && (bErrorStop==0) && (bUserStop==false)){
                QThread::msleep(1000);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                if (m_wsc.contains(clientIP)){
                    if (m_wsc.value(clientIP)->isConnected()){
                        break;
                    }
                }else{
                    bErrorStop = 1;
                    err = "ERROR: websocket "+clientIP+" not connected";
                    tp->setComment(err);
                    emit errorStop(2, err);
                    break;
                }
                itimeout--;
                emit updateStatus(" wait WSClient connect to client: "+s +
                                  " ("+ QString::number(itimeout) +")");
            }
            if (itimeout<=0){
                emit errorStop(1,"ERROR: Wait connect to " +s+ "timeout");
                break;
            }
            if(bErrorStop>0){
                emit errorStop(3, "Unknown error happen!!("+QString::number(bErrorStop)+")");
                return;
            }
            if(bUserStop){
                debug("[TpWorker]User Stop on wait client websocket connected!!");
                return;
            }
            //tell client add iperf client
            cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow);
            cmd = cmd + ":" + (m_ignoreWrongInterval?"1":"0");
            cmd = cmd + ":" +tp->getClientArgs();
            debug("[TpWorker]client cmd:" + clientIP + " => " + cmd, 5);
            rs = m_wsc[clientIP]->sendText(cmd);
            if (rs<=0){
                emit errorStop(2, "Setup client iperf config fail: "+ tp->getClientArgs());
                break;
            }
            QString ckey =tp->getBindKey(false);
            debug("[TpWorker]m_status_client ckey:" + ckey);
            m_status_client[ckey]=TPStatus::init;// init client of BindKey status 0
        } else {
            // qInfo() << "Ignore disabled TP test pair: " << tp;
        }

    }
    emit updateInterval(maxInterval);
    if(bErrorStop>0){
        emit errorStop(4, "Unknown error happen!!("+QString::number(bErrorStop)+")");
        return;
    }
    //Start server
    err = "[TpWorker]server websocket keys:" + m_wss.keys().join(" ");
    debug(err);
    foreach (QString key, m_wss.keys()){
        debug("[TpWorker]Let Server " + key + " CMD_IPERF_START "+ startTime + ":S");
        rs = m_wss[key]->sendText(QString(CMD_IPERF_START)+":"+startTime+":S");
        if (rs<=0){
            emit errorStop(3, "Start iperf server fail:" + key);
            break;
        }
    }
    if(bErrorStop>0){
        debug("[TpWorker]Start server error happen!!");
        return;
    }
    //###############################
    // QThread::sleep(5); // wait 5 sec
    //TODO: wait server start up and ready
    bool bServerReady = waitAllServerReady();

    if(bUserStop){
        err = "User Stop on wait ServerReady!!";
        emit updateStatus(err);
        return;
    }
    if (!bServerReady){
        QStringList ds;
        foreach (auto key, m_status_server.keys()){
            if (m_status_server.value(key)!=TPStatus::started){
                ds.append(key);
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        debug("[TpWorker]server not readey: " + ds.join(","),4);
        emit errorStop(4, "Iperf server not readey:" +  ds.join(","));
        return;
    }
    //Start client
    foreach (QString key, m_wsc.keys()){
        debug("[TpWorker]Let Client " + key + " CMD_IPERF_START " + startTime + ":C");
        rs = m_wsc[key]->sendText(QString(CMD_IPERF_START)+":"+startTime+":C");
        if (rs<=0){
            emit errorStop(4, "Start iperf client fail:" + key);
            break;
        }
    }
    bool bClientReady = waitAllClientReady();
    if (!bClientReady){
        QStringList ds;
        foreach (auto key, m_status_client.keys()){
            if (m_status_client.value(key)!=TPStatus::started){
                ds.append(key);
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        debug("[TpWorker]client not readey: " + ds.join(","),4);
        emit errorStop(4, "Iperf client not readey:" +  ds.join(","));
        return;
    }
    if(bErrorStop>0){
        debug("[TpWorker]Start client error happen!!");
        return;
    }
    QDateTime waitStartTime = QDateTime::currentDateTime();
    QDateTime waitEndTime = QDateTime::currentDateTime();
    qint64 iWait = waitStartTime.secsTo(waitEndTime);
    // QStringList dsserver;
    // QStringList dsclient;
    while (((iWait < (maxtestduration + m_extendwaittime)) || isRunforever) &&
           (bUserStop==false)){
        // while is a block func,
        // TODO: check all Servers/Client status in started or restarted
        // following may need more time to get working?
        // if (isStatusServersRunning(dsserver) && isStatusClientsRunning(dsclient)){
        //     // all are running
        // }else{
        //     err = QString("Some problem happen!! abort!! running server:%1, client:%2").arg(
        //         dsserver.join(","), dsclient.join(","));
        //     debug(err);
        //     //TODO: if set restartonerror, do not stop!!
        //     break;
        // }
        if ((getStatusServers()>m_status_server.keys().length()) ||
            (getStatusClients()>m_status_client.keys().length())) {
            err = "Some problem happen!! abort!! ";
            debug(err);
            break;
        }else if(getStatusServers()==0 && getStatusClients()==0) {
            debug("All test end, stop early");
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents); // not
        QThread::msleep(100); // block UI
        waitEndTime = QDateTime::currentDateTime();
        QString msg = "";
        if (isRunforever){
            msg = QString("Runtime %1 sec").arg(QString::number(iWait));
            if (iWait>60){
                msg = msg + "("+MyFunc::secToHumanReadable(iWait)+")";
            }
        }else{
            qint64 iRemain = (maxtestduration + m_extendwaittime)-iWait;
            msg = QString("Remain %1 sec").arg(QString::number(iRemain));
            if (iRemain>60){
                msg = msg + "("+MyFunc::secToHumanReadable(iRemain)+")";
            }
        }
        emit updateStatus(msg);
        iWait = waitStartTime.secsTo(waitEndTime);
    }
    err = "[TpWorker]iWait:" + QString::number(iWait) + "/" +
          QString::number(maxtestduration + m_extendwaittime)
          + " isRunforever:" + QChar('0' + isRunforever) + " bUserStop:" + QChar('0' +bUserStop);
    debug(err,4);
    onStop();
}

void TpWorker::initStart()
{
    bUserStop = false;
    emit testStarted();
    // ui->actionShowLog->setEnabled(true);
    // ui->actionSave->setEnabled(true);
    resetError();
    //TODO: clear old test record!!
    m_status_server.clear();
    m_status_client.clear();
}

void TpWorker::resetError()
{
    bErrorStop = 0;
    m_ErrorMSG = "";
}

void TpWorker::debug(QString msg, int debuglv)
{
    if (debuglv<=m_debuglv){
        emit debuginfo(msg);
    }
}

void TpWorker::onIperfStarted(QString smode, QString ipport)
{
    QMutexLocker<QMutex> locker(&m_mutex);
    debug("[TpWorker]onIperfStarted:" + smode + " : " + ipport, 4);
    if (smode.contains("S", Qt::CaseSensitive)){
        m_status_server[ipport] = TPStatus::started;
    }else{
        m_status_client[ipport] = TPStatus::started;
    }
}

void TpWorker::onIperfReStarted(QString smode, QString ipport)
{
    QMutexLocker<QMutex> locker(&m_mutex);
    // debug("onIperfReStarted:" + smode + " : " + ipport, 3);
    if (smode.contains("S", Qt::CaseSensitive)){
        m_status_server[ipport] = TPStatus::restarted;
        debug(ipport + " - m_status_server[ipport]:" +
              QString::number(static_cast<int>(m_status_server.value(ipport))));
    }else{
        m_status_client[ipport] = TPStatus::restarted;
        debug(ipport + " - m_status_client[ipport]:" +
              QString::number(static_cast<int>(m_status_client.value(ipport))));
    }

}

void TpWorker::onIperfStoped(QString refrow, QString err_no, QString err, QString ipport)
{
    debug("[TpWorker]onIperfStoped:" + refrow + " bind key: " + ipport +
                       " err_no:" + err_no + " err:" + err, 4);
    if (err_no.toInt()>0){
        emit updateComment(refrow, "["+ ipport +"]Error:" +err);
        if (m_status_server.contains(ipport)){
            m_status_server[ipport] = TPStatus::stoped;
        }
        if (m_status_client.contains(ipport)){
            m_status_client[ipport]=TPStatus::stoped;
        }
        //        emit errorStop(2, "onIperfStoped: ["+ipport+"]("+err_no+"):"+err);
    }else{

        // qiperf notify no error end:
        if (m_status_server.contains(ipport)){
            m_status_server[ipport]=TPStatus::init;
        }
        if (m_status_client.contains(ipport)){
            m_status_client[ipport]=TPStatus::init;
        }
    }
}
void TpWorker::onServerDisconnected(QString targetip)
{
    debug("[TpWorker]onServerDisconnected: " + targetip);
    // if (m_wss.contains(targetip)){
    //     m_wss.remove(targetip);
    // }
}

void TpWorker::onClientDisconnected(QString targetip)
{
    debug("[TpWorker]onClientDisconnected: " + targetip);
    // if (m_wsc.contains(targetip)){
    //     m_wsc.remove(targetip);
    // }
}

void TpWorker::onDisconnected(QString targetip)
{
    debug("[TpWorker]onDisconnected: " + targetip);
    if (m_wss.contains(targetip)){
        m_wss.remove(targetip);
    }
    if (m_wsc.contains(targetip)){
        m_wsc.remove(targetip);
    }
}

void TpWorker::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    // debug("[TpWorker]onIperfTPdata: " + refrow+ " sInterval:"+ sInterval + " datas:" + datas, 5);
    emit iperfTPdata(refrow, sInterval, datas);
}

void TpWorker::onIperfExtendWait(QString refrow, qint64 iwait, int exitCode)
{
    qint64 rs=0;
    m_extendwaittime = m_extendwaittime + iwait+10;
    debug("[TpWorker]onIperfExtendWait:" + refrow +
          " extend time:" + QString::number(iwait) +
          " m_extendwaittime:" + QString::number(m_extendwaittime), 3);
    QString stime = QDateTime::currentDateTime().toString(DATETIME_NOW_FORMAT);
    emit updateComment(refrow,
                       QString("[%1](%2)restart iperf")
                           .arg(stime,
                                QString::number(exitCode)));
                                // QString::number(m_extendwaittime)));
    QString binkey;
    // ask iperf server restart
    foreach (TP *tp, m_tps) {
        if (tp->getEnabled()){
            if (tp->row()==refrow.toInt()){
                QString key = tp->getServer();
                binkey = tp->getBindKey();
                debug(QString("[onIperfExtendWait]ask iperf server restart: %1, binkey: %2").arg(key, binkey),1);
                if (m_wss.contains(key)){
                    debug(QString("[onIperfExtendWait]ask %1 restart iperf server").arg(key), 1);
                    rs = m_wss[key]->sendText(QString(CMD_IPERF_RESTART)+":"+refrow+":S");
                    if (rs<=0){
                        emit errorStop(4, "[onIperfExtendWait]ReStart iperf server fail:" + key);
                        break;
                    }
                }
                if (!binkey.isEmpty()){
                    debug(QString("onIperfExtendWait:m_status_server: %1").arg(m_status_server.value(binkey)), 3);
                    m_flowManager->initialize(m_status_server);
                    m_flowManager->startMonitoringKey(binkey,
                                                      static_cast<TPStatus::Status>(m_status_server.value(binkey)), TPStatus::restarted, 10, bUserStop);
                }
                break;
            }
        }
    }

    // if (!binkey.isEmpty()){
    //     m_flowManager->startMonitoringKey(binkey, m_status_server[binkey], TPStatus::restarted, 10, bUserStop);
    // //     debug("m_status_server:" + m_status_server.keys().join(","));
    // //     waitServerReady(binkey, TPStatus::restarted);
    // }else{
    //     debug("No binkey", 3);
    //     QThread::sleep(3);
    // }
    //TODO: how to wait server restart ok?
    // QThread::msleep(500);
    // // ask iperf client restart
    // foreach (TP *tp, m_tps) {
    //     if (tp->getEnabled()){
    //         if (tp->row()==refrow.toInt()){
    //             QString key = tp->getClient();
    //             debug(QString("[onIperfExtendWait]TODO:ask iperf client restart: %1").arg(key),1);
    //             if (m_ws.contains(key)){
    //                 debug(QString("[onIperfExtendWait]ask %1 restart iperf client").arg(key), 1);
    //                 rs = m_ws[key]->sendText(QString(CMD_IPERF_RESTART)+":"+refrow+":C");
    //                 if (rs<=0){
    //                     emit errorStop(4, "ReStart iperf client fail:" + key);
    //                     break;
    //                 }
    //             }
    //             break;
    //         }
    //     }
    // }
}

bool TpWorker::isStatusServersRunning(QStringList &ds, bool bAll)
{
    //bAll: all status_server must be running;
    foreach (auto key, m_status_server.keys()){
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        if ((m_status_server.value(key)==static_cast<int>(TPStatus::started))||
            (m_status_server.value(key)==static_cast<int>(TPStatus::restarted))){
            if (!ds.contains(key)){
                ds.append(key);
            }
        }else{
            if (ds.contains(key)){
                qint64 idx = ds.indexOf(key);
                ds.remove(idx);
            }
        }
    }
    if (bAll){
        if (ds.count() == m_status_server.count()){
            return true;
        }else{
            return false;
        }
    }else{
        if (ds.count()>0){
            return true;
        }else{
            debug(QString("isStatusServersRunning:m_status_server: %1").arg(m_status_server.keys().join(",")));
            return false;
        }
    }
}

bool TpWorker::isStatusClientsRunning(QStringList &ds, bool bAll)
{
    //bAll: all status_server must be running;
    foreach (auto key, m_status_client.keys()){
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        if ((m_status_client.value(key)==static_cast<int>(TPStatus::started))||
            (m_status_client.value(key)==static_cast<int>(TPStatus::restarted))){
            if (!ds.contains(key)){
                ds.append(key);
            }
        }else{
            if (ds.contains(key)){
                qint64 idx = ds.indexOf(key);
                ds.remove(idx);
            }
        }
    }
    if (bAll){
        if (ds.count() == m_status_client.count()){
            return true;
        }else{
            return false;
        }
    }else{
        if (ds.count()>0){
            return true;
        }else{
            return false;
        }
    }
}

int TpWorker::getStatusServers()
{
    int sum = 0;
    // for (auto value : std::as_const(m_status_server)) {
    for (auto value : m_status_server.values()) {
        sum += value;
    }
    return sum;
}

int TpWorker::getStatusClients()
{
    int sum = 0;
    // for (auto value : std::as_const(m_status_client)) {
    for (auto value : m_status_client.values()) {
        sum += value;
    }
    return sum;
}

void TpWorker::onDebuginfo(QString msg)
{
    emit debuginfo(msg);
}

void TpWorker::provideCurrentStatus(const QString &key, QString smode)
{
    if (smode.startsWith("S")) {
        if (m_status_server.contains(key)) {
            int status = m_status_server.value(key);
            qDebug() << "TpWorker: Providing current status for" << key << ":" << status;
            // emit m_statusChecker->currentStatusReady(key, status);
        } else {
            qWarning() << "TpWorker: Requested status for unknown key:" << key;
            // emit m_statusChecker->currentStatusReady(key, -1);
        }
    }else{
        if (m_status_client.contains(key)) {
            int status = m_status_client.value(key);
            qDebug() << "TpWorker: Providing current status for" << key << ":" << status;
            // emit m_statusChecker->currentStatusReady(key, status);
        } else {
            qWarning() << "TpWorker: Requested status for unknown key:" << key;
            // emit m_statusChecker->currentStatusReady(key, -1);
        }
    }
}

void TpWorker::handleStatusAchieved(const QString &key, TPStatus::Status status)
{
    qDebug() << "HighLevelWorker: Received statusAchieved for" << key
             << " (Thread ID):" << QThread::currentThreadId();
    emit workFinished(QString("Success: %1 is %2").arg(key).arg(static_cast<int>(status)));

}

void TpWorker::handleTimeoutOccurred(const QString &key)
{
    qDebug() << "HighLevelWorker: Received timeoutOccurred for" << key
             << " (Thread ID):" << QThread::currentThreadId();
    emit workFinished(QString("Timeout: %1 did not reach desired status").arg(key));

}

void TpWorker::handleMonitoringFinished()
{
    qDebug() << "HighLevelWorker: TPStatusChecker's monitoringFinished (Thread ID):" << QThread::currentThreadId(); // Renamed debug output

}

void TpWorker::onStop(){
    QString cmd="";
    foreach (QString key, m_wss.keys()){
        WSClient *wsc = m_wss[key];
        if (wsc){
            cmd = QString(CMD_IPERF_STOP)+":" + key;
            debug("[TpWorker]"+ key + " m_ws send cmd: " + cmd);
            wsc->sendText(cmd);

            QThread::sleep(1);
            debug("[TpWorker]"+ key + " CMD_IPERF_CLEAR ",4);
            wsc->sendText(CMD_IPERF_CLEAR);
            wsc->close(); //close websocket
        }
    }

    bUserStop=true;
    // bStartTest = false;
    // updateRunStatus(false);
    emit testStoped(0);
    // QString endtime = QDateTime::currentDateTime().toString(DATETIME_NOW_FORMAT);
    // QDateTime enddatetime = QDateTime::fromString(endtime,DATETIME_NOW_FORMAT);
    QDateTime enddatetime = QDateTime::currentDateTime();
    QString endtime = enddatetime.toString(DATETIME_NOW_FORMAT);
    //TODO: error end message?

    // emit debuginfo("m_status_server:" + m_status_server + " m_status_client:" + m_status_client);
    qint64 consumetime = m_TestStartTime.secsTo(enddatetime);
    QString msg = "Finish at "+ endtime +" (Runtime: "+QString::number(consumetime)+" sec";
    if (consumetime>60){
        msg = msg +" ("+MyFunc::secToHumanReadable(consumetime)+")";
    }
    msg = msg + ")";
    emit updateStatus(msg);
    emit setEndTime(static_cast<double>(consumetime)); //update x-axis max value
}

void TpWorker::onSetStop()
{
    bUserStop = true;
}

void TpWorker::onStatusReady(QString refrow, const QString& skey,
                             TPStatus::Status status)
{
    debug(QString("(%1)onStatusReady: %2").arg(refrow, skey), 4);
    // ask iperf client restart
    qint64 rs = 0;
    foreach (TP *tp, m_tps) {
        if (tp->getEnabled()){
            if (tp->row()==refrow.toInt()){
                QString key = tp->getClient();
                debug(QString("[onIperfExtendWait]TODO:ask iperf client restart: %1").arg(key),1);
                if (m_wsc.contains(key)){
                    debug(QString("[onIperfExtendWait]ask %1 restart iperf client").arg(key), 1);
                    if (status==TPStatus::restarted){
                        rs = m_wsc[key]->sendText(QString(CMD_IPERF_RESTART) +
                                                 ":" + refrow + ":C");
                    }
                    if (rs<=0){
                        emit errorStop(4, "send CMD_IPERF_RESTART to iperf client fail:" + key);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void TpWorker::onWaitTimeout(QString refrow, const QString &skey)
{
    debug("onWaitTimeout:" + refrow + " skey:" + skey);
}
