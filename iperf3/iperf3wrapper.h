#ifndef IPERF3WRAPPER_H
#define IPERF3WRAPPER_H

#include <QObject>
// #include <QFuture>
// #include <QFutureWatcher>
#include <QHostAddress>
#include "iperf3worker.h"

// Forward declaration of the C structure to avoid including iperf_api.h here
struct iperf_test;

class Iperf3Wrapper : public QObject
{
    Q_OBJECT
public:
    explicit Iperf3Wrapper(QObject *parent = nullptr);
    ~Iperf3Wrapper();

    /**
     * @brief Starts an iPerf3 client test in a separate thread.
     * @param serverHost The IP address or hostname of the iPerf3 server.
     * @param port The port the server is listening on.
     * @param duration The test duration in seconds.
     * @param streams The number of parallel streams to use.
     * @param isClient act as client or server
     * @return true if the test was successfully initiated, false otherwise.
     */
    bool startTest(const QString& serverHost, int port, int duration,
                         int streams, bool isClient);
    void stopTest();
    /**
     * @brief The core blocking function that runs the iperf3 test.
     * This method is executed in a worker thread.
     * @param host The server host string.
     * @param port The server port.
     * @param duration The test duration.
     * @param streams The number of streams.
     * @param isClient act as client or server
     * @return The iperf3 JSON result string or an error message prefixed with "ERROR:".
     */
    static QString runIperfTest(const QString &host, int port, int duration,
                                int streams, bool isClient=true);
signals:
    void testStarted(quintptr threadId);
    /**
     * @brief Emitted when the test successfully completes.
     * @param jsonResult The raw JSON output string from iperf3.
     */
    void testFinished(const QString& jsonResult);

    /**
     * @brief Emitted when an error occurs during test setup or execution.
     * @param errorMessage A descriptive error message.
     */
    void testError(const QString& errorMessage);
    void testLog(const QString& message);
    void started();
    void StopTest();
    void iperfIntervalReport(const QList<StreamMetrics> &streamList);

private slots:
    void onTestStarted(quintptr threadId);
    // void onTestFinished();
    void onTestFinished(int resultCode);
    void onStarted();
    void onTestError(const QString& msg);
    void onTestOutput(const QString& msg);
    void onIperfIntervalReport(const QList<StreamMetrics> &streamList);
private:

    // QFuture<QString> m_future;
    // QFutureWatcher<QString> m_watcher;

    QThread *thread = nullptr;

    int json_handler(iperf_test *test);
};

#endif // IPERF3WRAPPER_H
