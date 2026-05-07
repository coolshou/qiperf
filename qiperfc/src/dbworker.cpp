#include "dbworker.h"
#include "comm.h"

#include <QDir>

DbWorker::DbWorker(QObject *parent)
    : QObject{parent}
{
    // 為每個執行緒產生唯一的連線名稱
    m_connectionName = QString("WorkerConnection_%1").arg(qintptr(this));
}

DbWorker::~DbWorker()
{
    flush();
    {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        db.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

void DbWorker::init(QString path)
{
    m_path = path;
}

void DbWorker::initDatabase()
{
    // 必須在 moveToThread 之後的 slot 中建立連線
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_path + QDir::separator() + QIPERF_TP_DATABASE);

    if (!db.open()) {
        qCritical() << "Database Error:" << db.lastError().text();
        return;
    }

    QSqlQuery q(db);
    // 高性能優化：開啟 WAL 模式
    q.exec("PRAGMA journal_mode=WAL;");
    q.exec("PRAGMA synchronous=NORMAL;");

    // 建立表格
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS tp_logs (
            refid TEXT,
            timestamp REAL,
            value REAL,
            packetlost REAL, packettotal REAL
            PRIMARY KEY (refid, timestamp)
        )";
    if (!q.exec(createTable)){
        qCritical() << "Create Table Failed:" << q.lastError().text();
    }
}

void DbWorker::handleData(TPDataPoint p)
{
    m_buffer.append(p);

    // 達到批次量就寫入
    if (m_buffer.size() >= m_batchSize) {
        writeToDb();
    }
}

void DbWorker::flush()
{
    if (!m_buffer.isEmpty()) {
        writeToDb();
    }
}

void DbWorker::writeToDb()
{
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen()) return;

    // 開始交易 (Transaction) - 這是 1kHz 存儲的效能關鍵
    db.transaction();

    QSqlQuery q(db);
    QString insertData = R"(
        INSERT INTO tp_logs (refid, timestamp, value, packetlost, packettotal)
            VALUES (:refid, :ts, :val, :lost, :tot))";
    q.prepare(insertData);
    for (const auto &p : m_buffer) {
        q.bindValue(":refid", p.refid);
        q.bindValue(":ts", p.timestamp);
        q.bindValue(":val", p.value);
        q.bindValue(":lost", p.packetlost);
        q.bindValue(":tot", p.packettotal);
        if (!q.exec()) {
            qWarning() << "Insert failed:" << q.lastError().text();
        }
    }

    if (db.commit()) {
        m_buffer.clear();
    } else {
        qCritical() << "Transaction commit failed!";
        db.rollback();
    }
}
