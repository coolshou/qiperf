#include "tpflowmanager.h"

#include <QCoreApplication>
#include <QEventLoop>

TpFlowManager::TpFlowManager(QObject *parent)
    : QObject{parent}
{
    m_debuglv = 3;
    m_pollingTimer = new QTimer(this);
    // set the periodic of timer (e.g., check every 200ms)
    m_pollingTimer->setInterval(200);
    // Connect the timer's timeout signal to our checking slot
    connect(m_pollingTimer, &QTimer::timeout, this, &TpFlowManager::checkStatusAndTimeout);

    // Connect the internal signals to the flow continuation slots
    connect(this, &TpFlowManager::StatusReady, this, &TpFlowManager::onReady);
    connect(this, &TpFlowManager::WaitTimeout, this, &TpFlowManager::onTimeout);

    // Initialize dummy server status for demonstration
    // m_status_server["myServerKey"] = STATUS_INITIAL;
}

TpFlowManager::~TpFlowManager()
{
    // QTimer is a child of MyFlowManager, so it will be deleted automatically.
    // Explicitly stopping is good practice if it could still be active.
    if (m_pollingTimer->isActive()) {
        m_pollingTimer->stop();
    }
}

void TpFlowManager::initialize(QMap<QString, TPStatus::Status> sharedStatusMap)
{
    m_sharedStatusMap = &sharedStatusMap;
}

void TpFlowManager::startMonitoringKey(QString key, TPStatus::Status skey,
                                       TPStatus::Status expectStatus,
                              qint64 waitTimeoutSeconds, bool& userstop)
{
    if (!m_sharedStatusMap) {
        debug("Error: TpFlowManager not initialized with a shared status map!");
        return;
    }

    if (m_activeMonitors.contains(key) && m_activeMonitors[key].isActive) {
        debug(QString("Monitoring for key '%1'(status:%2) is already active. Ignoring request.").arg(key, skey));
        return ;
    }
    debug(QString("Waiting for server '%1' (skey: %2)to reach status %3 (timeout: %4 s)").arg(key,
            QString::number(static_cast<int>(skey)),
            QString::number(static_cast<int>(expectStatus)),
            QString::number(waitTimeoutSeconds)));

    MonitorData data;
    data.startTime = QDateTime::currentDateTime();
    data.timeoutSeconds = waitTimeoutSeconds;
    data.expectedStatus = expectStatus;
    data.isActive = true;
    data.status = skey;

    m_activeMonitors.insert(key, data); // Add or update the monitoring data

    // m_skeyToWatch = key;
    // m_expectedStatus = expectStatus;
    // m_waitTimeoutSeconds = waitTimeoutSeconds;
    // m_waitStartTime = QDateTime::currentDateTime();
    m_bUserStop = userstop; // Reset user stop flag when starting a new wait

    // Ensure the global timer is running if it's the first active monitor
    if (!m_pollingTimer->isActive()) {
        m_pollingTimer->start();
    }


    // --- FOR DEMONSTRATION ONLY: Simulate server status change ---
    // Change status to STATUS_READY after 3 seconds
    // QTimer::singleShot(3000, this, [this]() {
    //     debug("Simulating server 'myServerKey' status change to STATUS_READY (1)...");
    //     m_status_server["myServerKey"] = TPStatus::Status::started;
    // });
    // You might also simulate timeout if it doesn't change
    // QTimer::singleShot( (waitTimeoutSeconds * 1000) + 1000, this, [this]() {
    //     debug("Simulating forced timeout for 'myServerKey'.");
    //     // This would ensure the timeout logic triggers if the status doesn't change
    // });
}

void TpFlowManager::stopMonitoringKey(const QString &skey)
{
    if (m_activeMonitors.contains(skey)) {
        m_activeMonitors.remove(skey);
        debug(QString("Stopped monitoring for key '%1' explicitly.").arg(skey));
        if (m_activeMonitors.isEmpty()) {
            m_pollingTimer->stop(); // Stop the timer if nothing else to monitor
            emit allMonitoringFinished();
        }
    }
}

void TpFlowManager::setRefrow(QString refrow)
{
    m_refrow = refrow;
}

void TpFlowManager::checkStatusAndTimeout()
{
    if (!m_sharedStatusMap) return; // Should not happen if initialized properly

    // Iterate over a copy of the keys to avoid issues if items are removed during iteration
    QList<QString> keysToRemove;

    for (auto it = m_activeMonitors.begin(); it != m_activeMonitors.end(); ++it) {
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        const QString& skey = it.key();
        MonitorData& data = it.value(); // Get a mutable reference
        // You might have a global m_bUserStop, or make it per-key within MonitorData
        // For simplicity, let's assume it's not handled here for multi-key.
        // If m_bUserStop is global, you'd check it here and add the key to keysToRemove
        // and emit serverWaitTimeout.

        qint64 elapsed = data.startTime.secsTo(QDateTime::currentDateTime());
        qint64 remaining = data.timeoutSeconds - elapsed;
        // Access the shared map via the pointer
        // FIXME: crash!!
        TPStatus::Status currentStatus = m_sharedStatusMap->value(skey); // Read from the shared map

        debug(QString("Monitoring (%1 sec remaining) server %2 status: %3 (expected: %4)")
                  .arg(QString::number(remaining),
                       skey,
                       QString::number(static_cast<int>(currentStatus)),
                       QString::number(data.expectedStatus)));

        // 1. Check if server status is ready
        if (currentStatus == data.expectedStatus) {
            debug(QString("Server '%1' is ready! Status: %2").arg(skey, QString::number(static_cast<int>(currentStatus))));
            keysToRemove.append(skey); // Mark for removal
            emit StatusReady(m_refrow, skey, currentStatus); // Trigger the next stage of the flow for THIS key
            // The `continueFlowAfterServerReady` slot will be called by the event loop later
        }
        // 2. Check for timeout
        else if (remaining <= 0) {
            debug(QString("[TpWorker]Wait Server %1 ready TIMEOUT").arg(skey), 3);
            keysToRemove.append(skey); // Mark for removal
            emit WaitTimeout(m_refrow, skey); // Trigger timeout handling for THIS key
            // The `cleanupAfterFlow` slot will be called by the event loop later
        }
    }

    // Remove finished/timed-out monitors AFTER iterating
    for (const QString& key : keysToRemove) {
        m_activeMonitors.remove(key);
    }

    // If no more active monitors, stop the global timer
    if (m_activeMonitors.isEmpty()) {
        m_pollingTimer->stop();
        debug("All active monitoring tasks completed. Global timer stopped.");
        emit allMonitoringFinished();
    }
}

void TpFlowManager::onReady(QString refrow, const QString& skey, TPStatus::Status status)
{
    debug(QString("(%1)flow continues: '%2' is now ready.(%3)").arg(refrow,
                                                                    skey,
                                                                    QString::number(static_cast<int>(status))));
    // --- This is where your "continues do following flow" logic goes ---
    // continueFlowAfterReady();
}

void TpFlowManager::onTimeout(const QString &skey)
{
    debug(QString("Main flow: Server '%1' wait timed out or user stopped.").arg(skey));
    // --- Handle the timeout case ---
    cleanupAfterFlow(); // Example: Clean up resources
}

void TpFlowManager::debug(const QString &msg, int level)
{
    if (level<=m_debuglv){
        emit debugMeg(QString("[TpFlowManager](%1)%2").arg(m_refrow, msg));
    }
}

void TpFlowManager::continueFlowAfterReady()
{
    emit FlowFinished(m_refrow); // Signal that the entire flow is done
}

void TpFlowManager::cleanupAfterFlow()
{
    debug("Cleaning up resources after flow (due to timeout or user stop).");
    // Perform any necessary cleanup here
    emit FlowFinished(m_refrow); // Signal that the flow finished (even if it was a timeout)

}
