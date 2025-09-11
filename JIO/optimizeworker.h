#ifndef OPTIMIZEWORKER_H
#define OPTIMIZEWORKER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>

class OptimizeWorker : public QObject
{
    Q_OBJECT
public:
    explicit OptimizeWorker(QJsonObject initdata, QObject *parent = nullptr);
public slots:
    void work();
    void Stop();
    void log(QString msg, int lv=3);
    void addIperf(QString server, QString client, int duration, int port=5201);
signals:
    void started();
    void stoped(int error);
    void debugMsg(QString msg);
    void sigAddIperf(QString cfg);
private:
    void setStop(bool stop);
    int mDebugLv;
    QJsonObject mInitData;
    QJsonArray mPosArr;
    bool mStop;
    QString mLocalAddr;
};

#endif // OPTIMIZEWORKER_H
