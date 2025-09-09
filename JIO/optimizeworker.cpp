#include "optimizeworker.h"

#include <QThread>

OptimizeWorker::OptimizeWorker(QJsonObject initdata, QObject *parent)
    : QObject{parent}, mInitData(initdata)
{
    mStop=false;
    mLocalAddr = mInitData["LocalAddr"].toString();
    mPosArr = mInitData["positions"].toArray();
}

void OptimizeWorker::work()
{
    // long run of Optimize work, run in Qthread!!
    mStop=false;
    emit started();
    qDebug() << "update item info to DlgOptimize";

    qDebug() << "// create iperf test pair in qiperfc";

    while (!mStop){

        //
        // TODO: adjuse CM7's BeamID/Att/...



        qDebug() << "//     => ask to run thrughput";
        qDebug() << "// wait throughput result";
        QThread::sleep(1000);
    }
}

void OptimizeWorker::Stop()
{
    setStop(true);
}

void OptimizeWorker::setStop(bool stop)
{
    mStop = stop;
}
