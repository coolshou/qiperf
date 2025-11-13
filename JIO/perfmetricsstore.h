#ifndef PERFMETRICSSTORE_H
#define PERFMETRICSSTORE_H

#include <QMap>
#include <QList>
#include <QPair>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QReadWriteLock>

class PerfMetricsStore {
public:
    void insert(const QString& timestamp, const QString& clientId, const QJsonObject& metrics) {
        /*/metrics
QJsonObject metrics;
metrics["AP_MCS"] = 9;
metrics["AP_RSSI"] = -65;
metrics["AP_SNR"] = 28.5;
metrics["AP_BeamID"] = 3;
metrics["MCS"] = 9;
metrics["RSSI"] = -65;
metrics["SNR"] = 28.5;
metrics["BeamID"] = 3;
metrics["UL"] = 120.5;
metrics["DL"] = 240.8;
        */
        QWriteLocker locker(&lock);
        timeIndex[timestamp][clientId] = metrics;
        clientIndex[clientId].append({ timestamp, metrics });
    }

    QMap<QString, QJsonObject> getClientsAt(const QString& timestamp) const {
        QReadLocker locker(&lock);
        return timeIndex.value(timestamp);
    }

    QList<QPair<QString, QJsonObject>> getClientHistory(const QString& clientId) const {
        QReadLocker locker(&lock);
        return clientIndex.value(clientId);
    }

    QJsonDocument toJson() const {
        QReadLocker locker(&lock);
        QJsonArray records;
        for (auto it = timeIndex.constBegin(); it != timeIndex.constEnd(); ++it) {
            QJsonObject record;
            record["timestamp"] = it.key();

            QJsonObject clients;
            for (auto clientIt = it.value().constBegin(); clientIt != it.value().constEnd(); ++clientIt) {
                clients[clientIt.key()] = clientIt.value();
            }

            record["clients"] = clients;
            records.append(record);
        }

        QJsonObject root;
        root["records"] = records;
        return QJsonDocument(root);
    }

private:
    mutable QReadWriteLock lock;
    QMap<QString, QMap<QString, QJsonObject>> timeIndex;
    QMap<QString, QList<QPair<QString, QJsonObject>>> clientIndex;
};

#endif // PERFMETRICSSTORE_H
