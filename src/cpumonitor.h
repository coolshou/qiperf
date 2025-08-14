#ifndef CPUMONITOR_H
#define CPUMONITOR_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QString>

// Platform-specific includes
#ifdef Q_OS_LINUX
#include <QFile>
#include <QTextStream>
#include <QStringList>
#elif defined(Q_OS_WIN)
#include <windows.h> // For GetSystemTimes
#elif defined(Q_OS_MAC)
#include <mach/mach_host.h>
#endif

class CpuMonitor : public QObject
{
    Q_OBJECT

public:
    explicit CpuMonitor(int interval=1, QObject *parent = nullptr);
    ~CpuMonitor();
    void setInterval(int interval);
    double getOverallCpuUsage() const { return overallCpuUsage; }

signals:
    void cpuUsageChanged(double percentage);
    void getCPUStatusFail();
private slots:
    void updateCpuUsage();
    void onGetCPUStatusFail();
private:
    QTimer *timer;
    int mInterval; //
    int mGetStatusFail; // continious fail
    double overallCpuUsage = 0.0;

    // Platform-specific members for storing previous CPU times
#ifdef Q_OS_LINUX
    long long prevTotalTime = 0;
    long long prevIdleTime = 0;
    bool readCpuTimes(long long &totalTime, long long &idleTime);
#elif defined(Q_OS_WIN)
    FILETIME prevIdleTime;
    FILETIME prevKernelTime;
    FILETIME prevUserTime;
    unsigned __int64 toULL(const FILETIME& ft); // Helper
#elif defined(Q_OS_MAC)
    natural_t prevCpuTicks[CPU_STATE_MAX] = {0};
    bool get_cpu_times(natural_t *cpuTicks);
#endif

    // Common initialization for platform-specific data
    void initializeCpuTimes();
};

#endif // CPUMONITOR_H
