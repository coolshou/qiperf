#include "tpstatuschecker.h"

TPStatusChecker::TPStatusChecker(QObject *parent) : QObject(parent)
{
    qDebug() << "TPStatusChecker constructor (Thread ID):" << QThread::currentThreadId();
    m_checkTimer = new QTimer(this);
    m_checkTimer->setInterval(500);
    m_checkTimer->setSingleShot(false);
    connect(m_checkTimer, &QTimer::timeout, this, &TPStatusChecker::onCheckTimeout);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(10000);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &TPStatusChecker::onOverallTimeout);
}

TPStatusChecker::~TPStatusChecker()
{
    qDebug() << "TPStatusChecker destructor (Thread ID):" << QThread::currentThreadId();
}

void TPStatusChecker::startMonitoring(const QString& keyToMonitor, TPStatus::Status desiredStatus)
{
    qDebug() << "TPStatusChecker::startMonitoring called (Thread ID):" << QThread::currentThreadId();
    m_keyToMonitor = keyToMonitor;
    m_desiredStatus = desiredStatus;

    qDebug() << "TSC: Starting to wait for key:" << m_keyToMonitor
             << "to be" << static_cast<int>(m_desiredStatus) << "with 10-second timeout.";

    stopMonitoring();
    m_checkTimer->start();
    m_timeoutTimer->start();
}

void TPStatusChecker::onCheckTimeout()
{
    emit requestCurrentStatus(m_keyToMonitor, "S");
}

void TPStatusChecker::currentStatusReady(const QString& key, int statusValue)
{
    if (key != m_keyToMonitor) {
        return;
    }

    if (statusValue == static_cast<int>(m_desiredStatus)) {
        qDebug() << "TSC: Status achieved for key:" << m_keyToMonitor << ". Value is:" << statusValue;
        stopMonitoring();
        emit statusAchieved(m_keyToMonitor, m_desiredStatus);
        emit monitoringFinished();
    } else {
        qDebug() << "TSC: Key:" << m_keyToMonitor << " current value:" << statusValue << ". Still waiting...";
    }
}


void TPStatusChecker::onOverallTimeout()
{
    qWarning() << "TSC: Timeout occurred for key:" << m_keyToMonitor << " after 10 seconds. Desired status was:" << static_cast<int>(m_desiredStatus);
    stopMonitoring();
    emit timeoutOccurred(m_keyToMonitor);
    emit monitoringFinished();
}

void TPStatusChecker::stopMonitoring()
{
    if (m_checkTimer->isActive()) {
        m_checkTimer->stop();
    }
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}
