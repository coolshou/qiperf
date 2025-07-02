#include "tpworker.h"

#include "comm.h"
#include <QDir>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>

#include "tpstatus.h"
#include "myfunc.h"

TpWorker::TpWorker(QString logpath, QList<TP *> &tps, int wstimeout,
                   int extrawait, int waitserverready,
                   QObject *parent)
    : QObject{parent}, m_logpath(logpath), m_tps(tps), iWSTimeout(wstimeout),
    iExtraWait(extrawait), m_WaitServerReady(waitserverready)
{
    m_debuglv = 3;
}

TpWorker::~TpWorker()
{

}

bool TpWorker::waitServerReady()
{
    QDateTime sTime = QDateTime::currentDateTime();
    bool bServerReady=false;
    // int waitedSecs = 0;

    while (!bServerReady && !bUserStop) {
        QThread::msleep(200);  // check 5 times per second
        QCoreApplication::processEvents();

        int readyCount = 0;
        for (const auto &skey : m_status_server.keys()) {
            if (m_status_server.value(skey) == TPStatus::started) {
                readyCount++;
            }
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

bool TpWorker::waitClientReady()
{
    QDateTime sTime = QDateTime::currentDateTime();
    bool bClientReady=false;
    // int waitedSecs = 0;

    while (!bClientReady && !bUserStop) {
        QThread::msleep(200);  // check 5 times per second
        QCoreApplication::processEvents();

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
    // list of throughput test pair
    foreach (TP *tp, m_tps) {
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
        if (tp->getEnabled()){
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
            debug(err);
            //if (!m_wss.contains(serverIP)) {
            if (!m_ws.contains(serverIP)) {
                //TODO: can not work with interface with DHCP under Windows??
                s = "ws://"+serverIP+":"+QString::number(QIPERFD_WSPORT);
                err =  "[TpWorker]server websocket:" + serverIP + " url: " + s + " m_datapath:" + m_datapath;
                debug(err);
                m_ws[serverIP]=new WSClient(serverIP, QUrl(s), m_datapath);
                connect(m_ws[serverIP], &WSClient::iperfStarted, this, &TpWorker::onIperfStarted);
                connect(m_ws[serverIP], &WSClient::iperfStoped, this, &TpWorker::onIperfStoped);
                // connect(m_ws[serverIP], &WSClient::disconnected, this, &TpWorker::onServerDisconnected);
                connect(m_ws[serverIP], &WSClient::disconnected, this, &TpWorker::onDisconnected);
                connect(m_ws[serverIP], &WSClient::iperfTPdata, this, &TpWorker::onIperfTPdata);
                connect(m_ws[serverIP], &WSClient::debuginfo, this, &TpWorker::onDebuginfo);
            }else{
                err =  "[TpWorker]m_wss exist:" + serverIP;
                debug(err);
                m_ws[serverIP]->setDatapath(m_datapath);
            }
            itimeout = iWSTimeout;
            while (itimeout>0 && (bErrorStop==0)&& (bUserStop==false)){
                QThread::msleep(1000);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                //m_wss.contains(serverIP) && ! m_wss[serverIP]->isConnected() &&
                if (m_ws.contains(serverIP)){
                    if (m_ws.value(serverIP)->isConnected()){
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
            cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getServerArgs();
            debug("[TpWorker]server cmd:" + serverIP + " => " + cmd);
            rs = m_ws[serverIP]->sendText(cmd);
            if (rs<=0){
                emit errorStop(1, "Setup server iperf config fail: "+ tp->getServerArgs());
                break;
            }
            QString skey = tp->getBindKey(true);
            debug("[TpWorker]skey:" + skey);
            m_status_server[skey]=TPStatus::init; // init server of BindKey status 0
            //###### client ######

            //RPC to control all client endpoint (iperf client)
            QString clientIP = tp->getMgrClient();
            err = "=====[TpWorker] client:" + clientIP;
            debug(err);
            //TODO: detect manager client is pingable
            if (!m_ws.contains(clientIP)) {
                s = "ws://"+clientIP+":"+QString::number(QIPERFD_WSPORT);
                debug("[TpWorker]client websocket:" + clientIP + " url: " + s + " m_datapath:" + m_datapath);
                m_ws[clientIP]=new WSClient(clientIP, QUrl(s), m_datapath);
                connect(m_ws[clientIP], &WSClient::iperfStarted, this, &TpWorker::onIperfStarted);
                connect(m_ws[clientIP], &WSClient::iperfStoped, this, &TpWorker::onIperfStoped);
                // connect(m_ws[clientIP], &WSClient::disconnected, this, &TpWorker::onClientDisconnected);
                connect(m_ws[clientIP], &WSClient::disconnected, this, &TpWorker::onDisconnected);
                connect(m_ws[clientIP], &WSClient::iperfTPdata, this, &TpWorker::onIperfTPdata);
                connect(m_ws[serverIP], &WSClient::debuginfo, this, &TpWorker::onDebuginfo);
            }else{
                debug("[TpWorker]m_wsc exist:" + clientIP);
                m_ws[clientIP]->setDatapath(m_datapath);
            }
            itimeout = iWSTimeout;
            while (itimeout>0 && (bErrorStop==0) && (bUserStop==false)){
                QThread::msleep(1000);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                if (m_ws.contains(clientIP)){
                    if (m_ws.value(clientIP)->isConnected()){
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
            cmd = QString(CMD_IPERF_ADD)+":"+QString::number(refrow)+":"+tp->getClientArgs();
            debug("[TpWorker]client cmd:" + clientIP + " => " + cmd);
            rs = m_ws[clientIP]->sendText(cmd);
            if (rs<=0){
                emit errorStop(2, "Setup client iperf config fail: "+ tp->getClientArgs());
                break;
            }
            QString ckey =tp->getBindKey(false);
            debug("[TpWorker]ckey:" + ckey);
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
    err = "[TpWorker]server keys:" + m_ws.keys().join(" ");
    debug(err);
    for (auto key: m_ws.keys()){
        debug("[TpWorker]Let Server " + key + " CMD_IPERF_START "+ startTime);
        rs = m_ws[key]->sendText(QString(CMD_IPERF_START)+":"+startTime+":S");
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
    bool bServerReady = waitServerReady();

    if(bUserStop){
        err = "User Stop on wait ServerReady!!";
        emit updateStatus(err);
        return;
    }
    if (!bServerReady){
        QStringList ds;
        foreach (auto key, m_status_server.keys()){
            if (m_status_server[key]!=TPStatus::started){
                ds.append(key);
            }
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
        debug("[TpWorker]server not readey: " + ds.join(","),4);
        emit errorStop(4, "Iperf server not readey:" +  ds.join(","));
        return;
    }
    //Start client
    for (auto key: m_ws.keys()){
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
        debug("[TpWorker]Let Client " + key + " CMD_IPERF_START " + startTime);
        rs = m_ws[key]->sendText(QString(CMD_IPERF_START)+":"+startTime+":C");
        if (rs<=0){
            emit errorStop(4, "Start iperf client fail:" + key);
            break;
        }
    }
    bool bClientReady = waitClientReady();
    if (!bClientReady){
        QStringList ds;
        foreach (auto key, m_status_client.keys()){
            if (m_status_client[key]!=TPStatus::started){
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
    while (((iWait < maxtestduration) || isRunforever) && (bUserStop==false)){
        if ((getStatusServers()>m_status_server.keys().length()) ||
            (getStatusClients()>m_status_client.keys().length())) {
            err = "Some problem happen!! abort!! ";
            debug(err);
            break;
        }else if(getStatusServers()==0 && getStatusClients()==0) {
            debug("All test end, stop early");
            break;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QThread::msleep(100);
        waitEndTime = QDateTime::currentDateTime();
        if (isRunforever){
            emit updateStatus("Runtime "+  QString::number(iWait) + " sec"
                              + "("+MyFunc::secToHumanReadable(iWait)+")");
        }else{
            emit updateStatus("Remain "+ QString::number(maxtestduration-iWait) + " sec");
        }

        iWait = waitStartTime.secsTo(waitEndTime);
    }
    err = "[TpWorker]iWait:" + QString::number(iWait) + "/" + QString::number(maxtestduration)
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
    if (debuglv>m_debuglv){
        emit debuginfo(msg);
    }
}

void TpWorker::onIperfStarted(QString smode, QString ipport)
{
    debug("[TpWorker]onIperfStarted:" + smode + " : " + ipport, 4);
    if (smode.contains("S", Qt::CaseSensitive)){
        m_status_server[ipport]=TPStatus::started;
    }else{
        m_status_client[ipport]=TPStatus::started;
    }
}

void TpWorker::onIperfStoped(QString refrow, QString err_no, QString err, QString ipport)
{
    debug("[TpWorker]onIperfStoped:" + refrow + " bind key: " + ipport +
                       " err_no:" + err_no + " err:" + err, 4);
    if (err_no.toInt()>0){
        emit updateComment(refrow, "["+ ipport +"]Error:" +err);
        if (m_status_server.contains(ipport)){
            m_status_server[ipport]=TPStatus::stoped;
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
    if (m_ws.contains(targetip)){
        m_ws.remove(targetip);
    }
}

void TpWorker::onIperfTPdata(QString refrow, QString sInterval, QString datas)
{
    emit iperfTPdata(refrow, sInterval, datas);
}
int TpWorker::getStatusServers()
{
    int sum = 0;
    // for (auto value : std::as_const(m_status_server)) {
    for (auto value : m_status_server) {
        sum += value;
    }
    return sum;
}

int TpWorker::getStatusClients()
{
    int sum = 0;
    // for (auto value : std::as_const(m_status_client)) {
    for (auto value : m_status_client) {
        sum += value;
    }
    return sum;
}

void TpWorker::onDebuginfo(QString msg)
{
    emit debuginfo(msg);
}

void TpWorker::onStop(){
    QString cmd="";
    foreach (auto key, m_ws.keys()){
        if (m_ws[key]){
            cmd = QString(CMD_IPERF_STOP)+":" + key;
            debug("[TpWorker]"+ key + " m_ws send cmd: " + cmd);
            m_ws[key]->sendText(cmd);
            // m_wsc[key]->close();
            QThread::sleep(1);
            debug("[TpWorker]"+ key + " CMD_IPERF_CLEAR ",4);
            m_ws[key]->sendText(CMD_IPERF_CLEAR);
        }
    }

    // Stop reg iperf client
    // foreach (auto key, m_wsc.keys()){
    //     if (m_wsc[key]){
    //         cmd = QString(CMD_IPERF_STOP)+":" + key;
    //         emit debuginfo("[TpWorker]"+key + " m_wsc send cmd: " + cmd);
    //         m_wsc[key]->sendText(cmd);
    //         m_wsc[key]->close();
    //     }
    //     // QCoreApplication::processEvents(QEventLoop::AllEvents);
    // }
    // // QThread::sleep(1);
    // // Stop reg iperf server
    // foreach (auto key, m_wss.keys()){
    //     if (m_wss[key]){
    //         cmd = QString(CMD_IPERF_STOP)+":" + key;
    //         emit debuginfo("[TpWorker]"+key + "m_wss send cmd: " + cmd);
    //         m_wss[key]->sendText(cmd);
    //         m_wss[key]->close();
    //     }
    //     // QCoreApplication::processEvents(QEventLoop::AllEvents);
    // }
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
    emit updateStatus("Finish at  "+ endtime +" (Runtime: "+QString::number(consumetime)+" sec"+
                      "("+MyFunc::secToHumanReadable(consumetime)+"))");
    emit setEndTime(static_cast<double>(consumetime)); //update x-axis max value
}

void TpWorker::onSetStop()
{
    bUserStop = true;
}
