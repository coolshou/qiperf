#include "memmonitor.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <QFile>
#include <QString>
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#else
#include <QRegularExpression>
#endif
#include <QTextStream>
#ifdef Q_OS_LINUX
#include <unistd.h> // for getpid()
#endif

#include <QDebug>

MemMonitor::MemMonitor(int interval, QObject *parent) :
    QObject(parent), mInterval(interval*1000)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MemMonitor::updateMemoryUsage);
    setInterval(interval);
}

MemMonitor::~MemMonitor()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

void MemMonitor::start(int intervalMs) {
    setInterval(intervalMs);
    timer->start(intervalMs);
}

void MemMonitor::stop() {
    timer->stop();
}

void MemMonitor::setInterval(int interval)
{
    mInterval = interval*1000;
    if (timer->isActive()) {
        timer->stop();
    }
    if (mInterval>0){
        timer->start(mInterval);
    }
}

void MemMonitor::updateMemoryUsage() {
    qint64 memKB = getMemoryUsageKB();
    qint64 memMB = round(memKB/1000);
    emit memoryUsageUpdated(memMB);
}

qint64 MemMonitor::getMemoryUsageKB() {
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024;
    }
#elif defined(Q_OS_LINUX)
    // qint64 pid = getpid();
    // QString path=QString("/proc/%1/status").arg(pid);
    // qDebug() << "Current PID:" << pid << " path:" << path;
    QString path=QString("/proc/self/status");
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        //readall
        QString data = in.readAll();
        file.close();
        foreach (QString line, data.split("\n")) {
            if (line.startsWith("VmRSS:")) {
                QStringList parts;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                parts = QString(line).split(QRegExp("\\s+"));
#else
                parts = QString(line).split(QRegularExpression("\\s+"));
#endif
                if (parts.size() >= 2) {
                    return parts[1].toLongLong(); // in KB
                }else{
                    qDebug() << "wrong format: " << line;
                }
            }
        }
    }else{
        qWarning() << "Cannot open file" << path << ":" << file.errorString();
        return file.error();
    }
#endif
    return -1;
}
