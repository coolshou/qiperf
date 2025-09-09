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

signals:
    void started();
    void stoped(int error);

private:
    void setStop(bool stop);
    QJsonObject mInitData;
    QJsonArray mPosArr;
    bool mStop;
    QString mLocalAddr;
};

#endif // OPTIMIZEWORKER_H
