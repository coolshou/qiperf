#ifndef TPFLOWMANAGER_H
#define TPFLOWMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QMap>

#include "tpstatus.h"
class TpFlowManager;

struct MonitorData {
    QDateTime startTime;
    qint64 timeoutSeconds;
    TPStatus::Status expectedStatus;
    // You might add a QTimer* here if you want a separate timer for each key,
    // but using a single shared timer that checks all active monitoring items
    // is often simpler and more efficient for many keys.
    // QTimer* uniqueTimer = nullptr; // Not used in this example, but an option

    // A flag to indicate if monitoring for this specific key is active
    bool isActive = false;
    TPStatus::Status status;
};

class TpFlowManager : public QObject
{
    Q_OBJECT
public:
    explicit TpFlowManager(QObject *parent = nullptr);
    ~TpFlowManager();
    // Using const QMap<QString, int>& means MyFlowManager can only read it.
    // If MyFlowManager *also* needed to modify it, you'd use QMap<QString, int>&
    void initialize(QMap<QString, TPStatus::Status> sharedStatusMap);

    // Additionally, connect to StatusManager's signal for immediate updates
    // void setStatusManager(StatusManager* manager);

    // The function to kick off the entire process
    void startMonitoringKey(QString key, TPStatus::Status skey,
                            TPStatus::Status expectStatus,
                   qint64 waitTimeoutSeconds, bool& userstop);
    // Function to stop monitoring a specific key (e.g., if user cancels)
    void stopMonitoringKey(const QString& skey);
    void setRefrow(QString refrow);
signals:
    // Signals to indicate the outcome of the wait
    void StatusReady(QString refrow, const QString& skey, TPStatus::Status status);
    void WaitTimeout(QString refrow, const QString& skey);
    void allMonitoringFinished(); // Optional: signal when all active monitoring stops
    void UserStoped(const QString& skey);
    void FlowFinished(QString refrow); // Signal when the entire flow completes
    void debugMeg(QString msg);

private slots:
    // Slot connected to the periodic timer to check status
    void checkStatusAndTimeout();

    // Slots for processing different stages of the flow
    void onReady(QString refrow, const QString& skey, TPStatus::Status status);
    void onTimeout(const QString& skey);

private:
    int m_debuglv;
    QString m_refrow;
    QTimer* m_pollingTimer;
    QDateTime m_waitStartTime;
    qint64 m_waitTimeoutSeconds;
    // A map to store the monitoring data for each active key
    QMap<QString, MonitorData> m_activeMonitors;

    QString m_skeyToWatch;
    // TPStatus::Status m_expectedStatus;
    // Pointer to the shared QMap. It does NOT own this map.
    // Making it a const pointer ensures MyFlowManager doesn't modify it directly.
    QMap<QString, TPStatus::Status>* m_sharedStatusMap = nullptr;

    // Pointer to the StatusManager to connect signals
    // StatusManager* m_statusManager = nullptr;

    // Simulate your m_status_server (replace with your actual QMap/source)
    // QMap<QString, int> m_status_server;
    bool m_bUserStop = false; // Your global user stop flag

    void debug(const QString& msg, int level = 3);
    // --- Your "following flow" functions ---
    void continueFlowAfterReady();
    void cleanupAfterFlow();

};

#endif // TPFLOWMANAGER_H
