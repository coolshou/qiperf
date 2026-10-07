#include "iperf3wrapper.h"
#include <QtConcurrent/QtConcurrent>
#include <QThread>

#include <QDebug>
#include "iperf3worker.h"

Iperf3Wrapper::Iperf3Wrapper(QObject *parent)
    : QObject(parent)
{

}

Iperf3Wrapper::~Iperf3Wrapper()
{
}
// Static function runs in a separate thread via QtConcurrent::run()
QString Iperf3Wrapper::runIperfTest(const QString &host, int port, int duration,
                                    int streams, bool isClient)
{
    struct iperf_test *test = nullptr;
    // Use a try/catch block to safely handle and report C API errors via QString
    try {
        test = (struct iperf_test *)iperf_new_test();
        if (!test)
            throw QString("Failed to create new iperf test: %1").arg(iperf_strerror(errno));
        if (iperf_defaults(test)){
            qDebug() << "set iperf_defaults ERROR!!";
            return "-1";
        }
        // test->forceflush = 1;
        iperf_set_test_forceflush(test);
        // Configure Test Settings
        iperf_set_test_server_port(test, port);
        if (isClient){
            iperf_set_test_server_hostname(test, host.toUtf8().constData());
            iperf_set_test_role(test, 'c');
            iperf_set_test_duration(test, duration);
            iperf_set_test_num_streams(test, streams);
        }else{
            // any address
            // qDebug() << "bind any address";
            // QString h="0.0.0.0";
            iperf_set_test_bind_address(test, host.toUtf8().constData());
            iperf_set_test_role(test, 's');
        }

        // set_protocol(test, Ptcp);
        //iperf_set_test_reverse(test, 0); //0,1
        //iperf_set_test_bidirectional(test, 0); //0,1
        //iperf_set_test_omit(test, 2);
        //iperf_set_test_logfile();
        //iperf_set_test_unit_format();
        //iperf_set_test_bind_address();
        //iperf_set_test_udp_counters_64bit();
        //iperf_set_test_mss();
        //iperf_set_on_new_stream_callback
        //iperf_set_on_test_start_callback
        //iperf_set_on_test_connect_callback
        //iperf_set_on_test_finish_callback


        // Set reporting interval to 1.0 seconds
        iperf_set_test_reporter_interval(test, 1.0);
        iperf_set_test_stats_interval(test, 1.0);

        // enable json report
        // iperf_set_test_json_output(test, 1);

        // Run the Test (This is the BLOCKING call)
        qDebug() << "currentThreadId:" << QThread::currentThreadId();
        // emit testStarted(QThread::currentThreadId());
        int run_result = 0;
        if (isClient){
            run_result = iperf_run_client(test);
        }else{
            // NOTICE: this will only accept a client connect, after that the test will nolonger exist!!
            run_result = iperf_run_server(test);
        }

        // Check for run failure
        if (run_result < 0) {
            throw QString("iPerf client run failed (Status Code: %1).").arg(run_result);
        }
        QString result = "";
        // 1. Trigger JSON Output generation and check its status
        // int json_status = iperf_get_test_json_output(test);
        // if (json_status < 0) {
        //     // If the generation itself fails, report it
        //     throw QString("iPerf JSON output generation failed (Status Code: %1).").arg(iperf_strerror(json_status));
        // }
        // // 2. Retrieve the generated JSON output string (using the corrected function name)
        // char *json_c_str = iperf_get_test_json_output_string(test);
        // if (!json_c_str)
        //     throw QString("iPerf test succeeded but retrieval function returned no JSON output.");

        // Convert C string to QString
        // QString result = QString::fromUtf8(json_c_str);

        // Cleanup and Return
        qDebug() << "finish test, cleanup";
        iperf_free_test(test);
        return result;

    } catch (const QString &error) {
        if (test) {
            iperf_free_test(test);
        }
        // Re-throw the QString exception to be caught by QFutureWatcher
        throw error;
    } catch (...) {
        if (test) {
            iperf_free_test(test);
        }
        throw QString("An unknown exception occurred during iperf test execution.");
    }
}

void Iperf3Wrapper::onTestStarted(quintptr threadId)
{
    emit testStarted(threadId);
}
bool Iperf3Wrapper::startTest(const QString& serverHost, int port,
                                    int duration, int streams, bool isClient)
{
    if (thread && thread->isRunning()) {
        // Stop it or return false
        thread->quit();
        thread->wait();
    }
    {
        //use QThread
        Iperf3Worker *worker = new Iperf3Worker();
        worker->config(serverHost, port, "TCP", duration, streams,
                       2, 1.0, false, false, isClient);
        thread = new QThread;
        worker->moveToThread(thread);

        connect(thread, &QThread::started, worker, &Iperf3Worker::startTest);
        // Connect cleanup slots
        connect(thread, &QThread::finished, worker, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);

        connect(worker, &Iperf3Worker::testStarted, this, &Iperf3Wrapper::onTestStarted);
        connect(worker, &Iperf3Worker::testOutput, this, &Iperf3Wrapper::onTestOutput);
        connect(worker, &Iperf3Worker::testFinished, this, &Iperf3Wrapper::onTestFinished);
        connect(worker, &Iperf3Worker::testError, this, &Iperf3Wrapper::onTestError);
        connect(worker, &Iperf3Worker::iperfIntervalReport, this, &Iperf3Wrapper::onIperfIntervalReport);

        connect(this, &Iperf3Wrapper::StopTest, worker, &Iperf3Worker::stopTest);
        thread->start();
    }
    return true;
}

void Iperf3Wrapper::stopTest()
{
    emit StopTest();
}

void Iperf3Wrapper::onStarted()
{
    emit started();
}

void Iperf3Wrapper::onTestError(const QString &msg)
{
    emit testError(msg);
}

void Iperf3Wrapper::onTestOutput(const QString &msg)
{
    emit testLog(msg);
}

void Iperf3Wrapper::onIperfIntervalReport(const QList<StreamMetrics> &streamList)
{
    emit iperfIntervalReport(streamList);
}

void Iperf3Wrapper::onTestFinished(int resultCode)
{
    thread->quit();
    thread->wait();
    emit testFinished(QString("iperf3 finished with code: %1").arg(resultCode));

}
int Iperf3Wrapper::json_handler(struct iperf_test *test) {
    const char* json_str = iperf_get_test_json_output_string(test);
    if (json_str) {
        qDebug() << "iperf JSON output:" << json_str;
        free((void*)json_str);
    }
    return 0;
}
