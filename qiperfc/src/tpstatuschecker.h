#ifndef TPSTATUSCHECKER_H
#define TPSTATUSCHECKER_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QDebug>
#include <QThread>

#include "tpstatus.h"

// enum class TPStatus {
//     stopped,
//     started,
//     restarted,
//     // ... other statuses
// };

class TPStatusChecker : public QObject // Renamed class
{
    Q_OBJECT

public:
    explicit TPStatusChecker(QObject *parent = nullptr); // Renamed constructor
    ~TPStatusChecker();

    void startMonitoring(const QString& keyToMonitor, TPStatus::Status desiredStatus);

signals:
    void requestCurrentStatus(const QString& key, QString smode);
    void currentStatusReady(const QString& key, int statusValue);

    void statusAchieved(const QString& key, TPStatus::Status status);
    void timeoutOccurred(const QString& key);
    void monitoringFinished();

private slots:
    void onCheckTimeout();
    void onOverallTimeout();
    void stopMonitoring();

private:
    QTimer *m_checkTimer;
    QTimer *m_timeoutTimer;
    QString m_keyToMonitor;
    TPStatus::Status m_desiredStatus;
};

#endif // TPSTATUSCHECKER_H
