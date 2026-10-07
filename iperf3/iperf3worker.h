#ifndef IPERF3WORKER_H
#define IPERF3WORKER_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <map>
#include <mutex>
#include <thread>

// --- C API Includes (Requires iperf3 library to be installed and linked) ---
extern "C" {
#include "iperf.h"
#include "iperf_api.h"
}
struct StreamMetrics {
    int streamId;
    double start_time;
    double end_time;
    long long bytesTransferred;
    long rtt; //retransmits
    double bandwidth; // Bits per second
    QString unit_format;
    int64_t interval_packet_count; // UDP total packet
    int64_t interval_cnt_error;     // UDP total error packet
    double lost_percent;            // UDP packet lost rate
    int omitted;
    bool isSender;
};

class Iperf3Worker : public QObject {
  Q_OBJECT
public:
  explicit Iperf3Worker(QObject *parent = nullptr);
  ~Iperf3Worker();

  int config(const QString &target, int port = 5201, QString protocal = "TCP",
             int duration = 10, int streams = 1, int omit = 2,
             double interval = 1.0, bool bidir = false, bool reverse = false,
             bool isClient = true, QString bindaddr = "0.0.0.0");
  int applyConfig(struct iperf_test *test);
  void startTest();
public slots:
  void stopTest();
signals:
  void testStarted(quintptr threadId);
  void testFinished(int resultCode);
  void testOutput(const QString &output);
  void testError(const QString &error);
  // report bandwidth data all at once
  void iperfIntervalReport(const QList<StreamMetrics> &streamList);

private:
  // bool running = false;
  bool mIsClient = true;
  bool bStop;
  // for callback
  static void onTestStartCallback(struct iperf_test *test);
  // iperf configs values
  QString mTarget;
  int mPort;
  QString mProtocal;
  int mDuration;
  int mStreams;
  int mOmit;
  double mInterval;
  bool mBidir;
  bool mReverse;
  QString mBindaddr;

  struct iperf_test *test = nullptr;
  std::thread iperfThread;
  QMutex testMutex; // protect access to test pointer
  std::atomic<bool> running{false};
  // The critical static C-style callback wrapper
  static void c_throughputCallback(struct iperf_test *test);
  // The internal dynamic C++ function that executes the actual work
  void handleThroughputReport(struct iperf_test *test);
  QString unitfmt;
  double dUnit;
  double getUnitFormat(char funit, QString &unitfmt);
};

#endif // IPERF3WORKER_H
