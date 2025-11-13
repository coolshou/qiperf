#include "optimizeworker.h"

#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>

OptimizeWorker::OptimizeWorker(QJsonObject initdata, QObject *parent)
    : QObject{parent}, mInitData(initdata)
{
    mDebugLv = 3;
    mStop=false;
    mLocalAddr = mInitData.value("LocalAddr").toString();
    mPosArr = mInitData.value("positions").toArray();
    mDuration = mInitData.value("TPDuration").toInt(30);
    //AP data

    for (QJsonArray::const_iterator it=mPosArr.constBegin(); it!=mPosArr.constEnd(); ++it) {
        QJsonObject jObj= it->toObject();
        if (jObj.value("type").toInt()==0){
            mAPAddr = jObj.value("IPAddr").toString();
            mAPMac = jObj.value("MacAddr").toString();
            QJsonObject jAIP1 = jObj.value("AIP1").toObject();
            mAIP1BeamID = jAIP1.value("APBeamID").toInt();
            QJsonObject jAIP2 = jObj.value("AIP2").toObject();
            mAIP2BeamID = jAIP2.value("APBeamID").toInt();
        }
    }
}

void OptimizeWorker::work()
{
    // long run of Optimize work, run in Qthread!!
    mStop=false;
    emit started();
    log("update item info to DlgOptimize");
    int port=5201;
    QString clientaddr;
    QString clientname;
    int apbeamid=0;
    currentDateTime = QDateTime(QDateTime::currentDateTime());
    //create iperf test pair to each client on qiperf console
    for (QJsonArray::const_iterator it=mPosArr.constBegin(); it!=mPosArr.constEnd(); ++it) {
        QJsonObject jObj= it->toObject();
        if (jObj.value("type").toInt()==1){
            //client
            clientaddr = jObj.value("IPAddr").toString();
            log("// create iperf test pair in qiperfc:" +mLocalAddr+ " <=> " + clientaddr);
            clientname = jObj.value("name").toString();
            addIperf(mLocalAddr, clientaddr, mDuration, port);
            QJsonObject jAIP1 = jObj.value("AIP1").toObject();
            QString aipgp = jAIP1.value("aipgroup").toString();
            if (aipgp.contains("AIP1")){
                apbeamid = mAIP1BeamID;
            }else{
                apbeamid = mAIP2BeamID;
            }
            int clientbeamid = jAIP1.value("ClientBeamID").toInt();
            //show test data on DlgOptimize
            emit sigAddData(currentDateTime, apbeamid, clientname, clientbeamid);

        }
        port++;
    }
    qDebug() << "//ask run iperf";

    qDebug() << "//wait iperf result";

    //clean iperf
    emit sigClearIperf();


    while (!mStop){

        //
        // TODO: adjuse CM7's BeamID/Att/...

        // if (!Var1) {
        //     QEventLoop loop;
        //     connect(this, &OptimizeWorker::var1Changed, [&]() {
        //         if (Var1) loop.quit();
        //     });
        //     loop.exec();
        // }

        log("//     => ask to run thrughput");
        log("// wait throughput result");
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        QThread::sleep(1); // sleep 1 sec
    }
    emit stoped(0);
}

void OptimizeWorker::Stop()
{
    setStop(true);
}

void OptimizeWorker::log(QString msg, int lv)
{
    if (lv>=mDebugLv){
        emit debugMsg(msg);;
    }
}

void OptimizeWorker::addIperf(QString server, QString client, int duration, int port)
{
    QString cfg =QString("{\"Action\":\"IPERF_ADD\",\"client\":{\"bidir\":false,\"bind\":\"%2\",\"bitrate\":0,\"buffer\":0,\"delaytime\":0,\"dscp\":-1,\"duration\":%3,\"fmtreport\":\"m\",\"interval\":1,\"ipv6\":false,\"manager\":\"%2\",\"mss\":0,\"omit\":2,\"parallel\":1,\"port\":%4,\"protocal\":\"TCP\",\"restartonerror\":false,\"reverse\":false,\"target\":\"%1\",\"tos\":-1,\"unit_bitrate\":\"K\",\"unit_buffer\":\"\",\"unit_windowsize\":\"K\",\"version\":\"3\",\"windowsize\":0,\"zerocopy\":false},\"enabled\":true,\"server\":{\"bidir\":false,\"bind\":\"%1\",\"delaytime\":0,\"duration\":%3,\"fmtreport\":\"m\",\"interval\":1,\"manager\":\"%1\",\"parallel\":1,\"port\":%4,\"protocal\":\"TCP\",\"restartonerror\":false,\"reverse\":false,\"unit_windowsize\":\"K\",\"version\":\"3\",\"windowsize\":0}}").arg(server, client, QString::number(duration), QString::number(port));

    emit sigAddIperf(cfg);
}

void OptimizeWorker::setStop(bool stop)
{
    mStop = stop;
}
