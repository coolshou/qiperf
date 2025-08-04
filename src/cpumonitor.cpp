#include "cpumonitor.h"

#include <QRegularExpression>

CpuMonitor::CpuMonitor(QObject *parent) : QObject(parent)
{
    mGetStatusFail = 0;
    connect(this, &CpuMonitor::getCPUStatusFail, this, &CpuMonitor::onGetCPUStatusFail);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &CpuMonitor::updateCpuUsage);

    initializeCpuTimes(); // Call platform-specific initial time capture

    // Start the timer to update every 1 second (adjust as needed)
    timer->start(1000);
}

CpuMonitor::~CpuMonitor()
{
    if (timer->isActive()) {
        timer->stop();
    }
}

void CpuMonitor::initializeCpuTimes()
{
#ifdef Q_OS_LINUX
    readCpuTimes(prevTotalTime, prevIdleTime);
#elif defined(Q_OS_WIN)
    GetSystemTimes(&prevIdleTime, &prevKernelTime, &prevUserTime);
#elif defined(Q_OS_MAC)
    get_cpu_times(prevCpuTicks);
#else
    qWarning() << "CPU monitoring not implemented for this OS.";
#endif
}

void CpuMonitor::updateCpuUsage()
{
    // Common variables for calculation
    long long currentTotalTime = 0;
    long long currentIdleTime = 0;
    bool success = false;
#ifdef Q_OS_WIN
    // Declare Windows-specific variables at a wider scope
    // so they can be used when updating prev values.
    unsigned __int64 currentIdleULL = 0;
    unsigned __int64 currentKernelULL = 0;
    unsigned __int64 currentUserULL = 0;
#endif

#ifdef Q_OS_LINUX
    success = readCpuTimes(currentTotalTime, currentIdleTime);
#elif defined(Q_OS_WIN)
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        currentIdleULL = toULL(idleTime);
        currentKernelULL = toULL(kernelTime);
        currentUserULL = toULL(userTime);

        // Windows values are in 100-nanosecond units.
        // We'll treat them as "ticks" for consistency in calculation logic.
        currentIdleTime = currentIdleULL;
        currentTotalTime = currentKernelULL + currentUserULL; // Kernel includes idle, so this sum is total.
        success = true;
    }
#elif defined(Q_OS_MAC)
    natural_t currentCpuTicks[CPU_STATE_MAX];
    if (get_cpu_times(currentCpuTicks)) {
        for (int i = 0; i < CPU_STATE_MAX; ++i) {
            currentTotalTime += currentCpuTicks[i];
        }
        currentIdleTime = currentCpuTicks[CPU_STATE_IDLE];
        success = true;
    }
#else
    qWarning() << "CPU monitoring not implemented for this OS.";
    return; // Exit if not implemented
#endif

    if (success) {
        long long totalDiff = currentTotalTime -
                              (
#ifdef Q_OS_LINUX
                                  prevTotalTime
#elif defined(Q_OS_WIN)
                                  toULL(prevKernelTime) + toULL(prevUserTime)
#elif defined(Q_OS_MAC)
                                  prevCpuTicks[CPU_STATE_USER] + prevCpuTicks[CPU_STATE_SYSTEM] + prevCpuTicks[CPU_STATE_IDLE] + prevCpuTicks[CPU_STATE_NICE]
#endif
                              );

        long long idleDiff = currentIdleTime -
                             (
#ifdef Q_OS_LINUX
                                 prevIdleTime
#elif defined(Q_OS_WIN)
                                 toULL(prevIdleTime)
#elif defined(Q_OS_MAC)
                                 prevCpuTicks[CPU_STATE_IDLE]
#endif
                             );

        if (totalDiff > 0) {
            overallCpuUsage = 100.0 * (static_cast<double>(totalDiff - idleDiff) / totalDiff);
        } else {
            overallCpuUsage = 0.0; // Avoid division by zero
        }

        // Store current times as previous for the next iteration
#ifdef Q_OS_LINUX
        prevTotalTime = currentTotalTime;
        prevIdleTime = currentIdleTime;
#elif defined(Q_OS_WIN)
        prevIdleTime.dwLowDateTime = (DWORD)(currentIdleTime & 0xFFFFFFFF);
        prevIdleTime.dwHighDateTime = (DWORD)(currentIdleTime >> 32);
        prevKernelTime.dwLowDateTime = (DWORD)(currentKernelULL & 0xFFFFFFFF);
        prevKernelTime.dwHighDateTime = (DWORD)(currentKernelULL >> 32);
        prevUserTime.dwLowDateTime = (DWORD)(currentUserULL & 0xFFFFFFFF);
        prevUserTime.dwHighDateTime = (DWORD)(currentUserULL >> 32);
#elif defined(Q_OS_MAC)
        for (int i = 0; i < CPU_STATE_MAX; ++i) {
            prevCpuTicks[i] = currentCpuTicks[i];
        }
#endif

        emit cpuUsageChanged(overallCpuUsage);
        // qDebug() << "CPU Usage: " << QString::number(overallCpuUsage, 'f', 2) << "%"; // Uncomment for debug
    } else {
        emit getCPUStatusFail();
        qWarning() << "Failed to get CPU times for current OS.";
    }
}

void CpuMonitor::onGetCPUStatusFail()
{
    mGetStatusFail = mGetStatusFail + 1;
    if (mGetStatusFail>10){
        //when error 10 times, stop
        if (timer->isActive()){
            emit cpuUsageChanged(-1);
            timer->stop();
        }
    }
}

// --- Platform-specific implementations ---

#ifdef Q_OS_LINUX
bool CpuMonitor::readCpuTimes(long long &totalTime, long long &idleTime)
{
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    QString line = in.readLine();
    file.close();
    if (line.isEmpty() || !line.startsWith("cpu")) {
        qDebug() << "Invalid or missing CPU line:" << line;
        return false;
    }
    static const QRegularExpression whitespaceRegExp("\\s+");
    QStringList parts = line.split(whitespaceRegExp, Qt::SkipEmptyParts);
    // Minimum parts: cpu, user, nice, system, idle
    if (parts.size() >= 5) {
        quint64 user = parts[1].toULongLong();
        quint64 nice = parts[2].toULongLong();
        quint64 system = parts[3].toULongLong();
        quint64 idle = parts[4].toULongLong();
        // These fields might not always be present depending on kernel version
        quint64 iowait = (parts.size() >= 6) ? parts[5].toULongLong() : 0;
        quint64 irq = (parts.size() >= 7) ? parts[6].toULongLong() : 0;
        quint64 softirq = (parts.size() >= 8) ? parts[7].toULongLong() : 0;
        quint64 steal = (parts.size() >= 9) ? parts[8].toULongLong() : 0;
        quint64 guest = (parts.size() >= 10) ? parts[9].toULongLong() : 0;
        quint64 guest_nice = (parts.size() >= 11) ? parts[10].toULongLong() : 0;

        idleTime = idle + iowait; // iowait is typically considered idle time
        totalTime = user + nice + system + idle + iowait + irq + softirq + steal + guest + guest_nice;
        return true;
    }
    return false;
}
#endif // Q_OS_LINUX

#ifdef Q_OS_WIN
unsigned __int64 CpuMonitor::toULL(const FILETIME& ft)
{
    return (static_cast<unsigned __int64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}
#endif // Q_OS_WIN

#ifdef Q_OS_MAC
bool CpuMonitor::get_cpu_times(natural_t *cpuTicks)
{
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t kr = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)cpuTicks, &count);
    return (kr == KERN_SUCCESS);
}
#endif // Q_OS_MAC
