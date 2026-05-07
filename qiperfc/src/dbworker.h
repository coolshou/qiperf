#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QVector>
#include <QString>
#include <QDebug>

// 定義數據結構
struct TPDataPoint {
    QString refid;  // ref id
    double timestamp; // x
    double value;  // y
    qint64 packetlost; // packetlost
    qint64 packettotal; //packettotal
};

// database worker to write throughput record in background
class DbWorker : public QObject
{
    Q_OBJECT
public:
    explicit DbWorker(QObject *parent = nullptr);
    ~DbWorker();

public slots:
    void init(QString path);
    // 初始化資料庫（在 Thread 開始後執行）
    void initDatabase();
    // 接收單點數據並緩存
    void handleData(TPDataPoint p);
    // 強制寫入目前緩存（例如程式關閉前）
    void flush();
signals:

private:
    QString m_path;
    void writeToDb(); // 執行實質的 SQL Transaction

    QVector<TPDataPoint> m_buffer;
    const int m_batchSize = 500; // 累積 500 點寫入一次 (約 0.5 秒)
    QString m_connectionName;
};

#endif // DBWORKER_H
