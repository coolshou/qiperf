#include "iperf3worker.h"

#include <QDebug>
#include <QThread>
#include <QVariant>
#include <unistd.h> // close()

#include "../iperf3/iperf3/src/queue.h"


Iperf3Worker::Iperf3Worker(QObject *parent) : QObject{parent} { bStop = false; }

Iperf3Worker::~Iperf3Worker() {
  // Ensure thread stops
  stopTest();
  if (iperfThread.joinable())
    iperfThread.join();

  // QMutexLocker locker(&testMutex);
  // if (test) {
  //     iperf_free_test(test);
  //     test = nullptr;
  // }
}

int Iperf3Worker::config(const QString &target, int port, QString protocal,
                         int duration, int streams, int omit, double interval,
                         bool bidir, bool reverse, bool isClient,
                         QString bindaddr) {
  emit testOutput("Iperf3Worker::config");
  QMutexLocker locker(&testMutex);
  mTarget = target;
  mPort = port;
  mProtocal = protocal;
  mDuration = duration;
  mStreams = streams;
  mOmit = omit;
  mInterval = interval;
  mBidir = bidir;
  mReverse = reverse;
  mIsClient = isClient;
  mBindaddr = bindaddr;
  return 0;
}

int Iperf3Worker::applyConfig(iperf_test *test) {
  int rc = 0;
  if (iperf_defaults(test)) {
    // reset to default
    emit testError("set iperf_defaults ERROR!!");
    return -1;
  }
  // emit testOutput("iperf set forceflush");
  iperf_set_test_forceflush(test);
  // emit testOutput("iperf set server port:"+ QString::number(mPort));
  iperf_set_test_server_port(test, mPort);
  // emit testOutput("iperf set bind address:"+ mBindaddr);
  iperf_set_test_bind_address(test, mBindaddr.toUtf8().constData());

  iperf_set_test_unit_format(test, 'm'); //Mbits/sec
  dUnit = getUnitFormat('m', unitfmt);
  // iperf_set_test_unit_format(test, 'M');    //MByte/sec

  // Set reporting interval to 1.0 seconds
  // emit testOutput("iperf set reporter interval:"+
  // QString::number(mInterval));
  // iperf_set_test_stats_interval(test, mInterval);
  // iperf_set_test_reporter_interval(test, mInterval);
  test->user_data = this; // Pass 'this' Qt instance pointer to the C context
  iperf_set_throughput_report_callback(test,
                                       Iperf3Worker::c_throughputCallback);
  if (mIsClient) {
    // emit testOutput("iperf set client mode");
    iperf_set_test_role(test, 'c');
    // emit testOutput("iperf set server hostname:"+ mTarget);
    iperf_set_test_server_hostname(test, mTarget.toUtf8().constData());
    // emit testOutput("iperf set test duration:"+ QString::number(mDuration));
    iperf_set_test_duration(test, mDuration);
    // emit testOutput("iperf set num streams:"+ QString::number(mStreams));
    iperf_set_test_num_streams(test, mStreams);
    // emit testOutput("iperf set omit:"+ QString::number(mOmit));
    iperf_set_test_omit(test, mOmit);

    if (mProtocal == "TCP") {
      rc = set_protocol(test, Ptcp);
    } else if (mProtocal == "UDP") {
      rc = set_protocol(test, Pudp);
    } else {
      qDebug() << "TODO: check sctp setting??";
      rc = set_protocol(test, Psctp);
    }
    QString result = "Fail";
    if (rc == 0) {
      result = "OK";
    }
    // emit testOutput("iperf set protocol:"+ mProtocal+ " " + result);

    if (mBidir) {
      // emit testOutput("iperf set bidirectiona mode");
      iperf_set_test_bidirectional(test, 1); // 0,1
    }
    if (mReverse) {
      // emit testOutput("iperf set reverse mode");
      iperf_set_test_reverse(test, 1); // 0,1
    }
  } else {
    // emit testOutput("iperf set server mode");
    iperf_set_test_role(test, 's');
  }
  //
  // windowsize
  // bitrate
  // unit_bitrate
  // buffer
  // unit_buffer
  // dscp
  // mss
  // tos
  // format to report: Kbits, Mbits, Gbits, Tbits
  // timestamps
  // zerocopy

  // iperf_set_test_logfile();
  //  void	iperf_set_test_timestamps( struct iperf_test* ipt, int
  //  timestamps ); void	iperf_set_test_timestamp_format( struct
  //  iperf_test*, const char *tf );

  // iperf_set_test_udp_counters_64bit();
  // iperf_set_test_mss();
  // iperf_set_test_tos();

  // iperf_set_on_new_stream_callback

  // following callback not work!!
  // iperf_set_on_test_start_callback(test, onTestStartCallback);
  // iperf_set_on_test_connect_callback(test, onTestStartCallback);
  // iperf_set_on_test_finish_callback(test, onTestStartCallback);

  // iperf_set_test_json_output(test, 0);  // text output (default)
  // iperf_set_test_json_output(test, 1);  // json output
  return 0;
}

void Iperf3Worker::startTest() {

  // if (!test || running.load()) return;
  if (running.load()) {
    emit testOutput("running.loaded!!");
    return;
  }
  bStop = false;
  running.store(true);

  emit testStarted(reinterpret_cast<quintptr>(QThread::currentThreadId()));
  // start iperf_run_client in a separate std::thread so this QObject's
  // thread remains responsive to slots (stopTest).
  iperfThread = std::thread([this]() {
    int rc = 0;
    do {
      struct iperf_test *local_test = iperf_new_test();
      if (!local_test) {
        return;
      }
      { // Use a mutex if 'test' is accessed by other threads
        QMutexLocker locker(&testMutex);
        test = local_test;
      }
      applyConfig(local_test);
      if (mIsClient) {
        emit testOutput("iperf client connect to " + mTarget);
        rc = iperf_run_client(local_test); // BLOCKING call here
        if (rc < 0) {
          emit testError(QString::fromUtf8(iperf_strerror(i_errno)));
        }
        bStop = true; //.store(true); // Clients usually only run once
      } else {
        emit testOutput("iperf server wait for connect, run forever:" +
                        QVariant(!bStop).toString());
        rc = iperf_run_server(local_test); // this only accept a client connect
        if (rc < 0) {
          emit testError("Server Error: " +
                         QString::fromUtf8(iperf_strerror(i_errno)));
          iperf_free_test(local_test);
          break;
        }
        // 正常結束一次連線，釋放本次的資源
        emit testOutput("iperf server stoped");
        iperf_free_test(local_test);
        emit testOutput("Connection finished, restarting server...");
      }
    } while (!bStop);

    running.store(false);
    emit testFinished(rc);
  });
}

void Iperf3Worker::stopTest() {
  // This slot runs in the worker's thread (event loop active).
  // It will be able to execute because iperf_run_client runs in iperfThread.
  if (!running.load())
    return;
  bStop = true;
  QMutexLocker locker(&testMutex);
  if (test) {
    qDebug() << "stopTest";
    iperf_stop_test(test);
  }
  qDebug() << "testFinished";
  // emit testFinished(-1);
  // join the blocking thread (do not block the GUI thread — this is a slot in
  // worker thread, so it's okay to wait here. If you want non-blocking join,
  // emit a signal to cleanup later.)
  if (iperfThread.joinable()) {
    qDebug() << "iperfThread join";
    iperfThread.join();
  }
}

void Iperf3Worker::onTestStartCallback(iperf_test *test) {
  Q_UNUSED(test)
  qDebug() << "onTestStartCallback";
}

void Iperf3Worker::c_throughputCallback(struct iperf_test *test) {
  if (test && test->user_data) {
    // Cast the opaque user_data back into our Qt object context
    Iperf3Worker *worker = static_cast<Iperf3Worker *>(test->user_data);

    // Forward execution out of the static context into our member function
    worker->handleThroughputReport(test);
  }
}
void Iperf3Worker::handleThroughputReport(struct iperf_test *test) {
    QList<StreamMetrics> currentMetrics;
    // collect all stream's data
    // id, interval, banswidth,
    // Extract calculated throughput data out of the iperf structure
    // (Note: Struct field names vary based on version, e.g., using test->getting
    // methods or variables) For illustration, let's assume we extract bits per
    // second and active interval:
    // Example: pulling from the last completed stream interval
    struct iperf_stream *sp = NULL;
    struct iperf_stream_result *res;
    struct iperf_interval_results *irp = NULL;
    struct iperf_time temp_time;
    // double start_time, end_time;

    // SLIST_FOREACH(iterator, head_pointer, field_name)
    SLIST_FOREACH(sp, &test->streams, streams) {
      StreamMetrics metrics;
      metrics.streamId = sp->id;
      // metrics.unit_format = test->settings->unit_format;
      metrics.unit_format = unitfmt;
      metrics.isSender = (sp->sender == 1);
      // 1. Get the stream's result structure
      res = sp->result;
      if (res) {
          // 2. Get the very last (latest) interval result from the TAILQ list
          // irp = TAILQ_LAST(&res->interval_results, irlisthead);
          struct iperf_interval_results *current_irp;
          TAILQ_FOREACH(current_irp, &res->interval_results, irlistentries) {
              irp = current_irp;
          }
          if (irp){
              metrics.omitted = irp->omitted;
              iperf_time_diff(&irp->interval_start_time, &irp->interval_end_time,
                              &temp_time);
              double interval_len = iperf_time_in_secs(&temp_time);
              if (interval_len >= test->stats_interval * 0.10 ||
                  irp->bytes_transferred > 0) {
                  iperf_time_diff(&res->start_time , &irp->interval_start_time, &temp_time);
                  metrics.start_time = iperf_time_in_secs(&temp_time);
                  iperf_time_diff(&res->end_time , &irp->interval_start_time, &temp_time);
                  metrics.end_time = iperf_time_in_secs(&temp_time);
                  if (metrics.end_time < test->stats_interval * 0.10){
                      //ignore last interval which interval is very small
                      continue;
                  }
                  //this time interval is ok
                  if (irp->interval_duration > 0.0) {
                      //all protocal : bandwidth
                      metrics.bytesTransferred = irp->bytes_transferred;
                      //byte to bit
                      metrics.bandwidth = (8 * (double)irp->bytes_transferred / (double)irp->interval_duration)/ dUnit;
                      // OPTIONAL: Handle TCP Retransmits (Only applies to TX streams)
                      if (metrics.isSender && test->protocol->id == Ptcp) {
                          metrics.rtt = irp->rtt;
                      }
                      if (test->protocol->id == Pudp){
                          //UDP: packet lost
                          metrics.interval_packet_count = irp->interval_packet_count;
                          metrics.interval_cnt_error = irp->interval_cnt_error;
                          if (metrics.interval_packet_count>0){
                              metrics.lost_percent = 100.0 * metrics.interval_cnt_error / metrics.interval_packet_count;
                          }else{
                              metrics.lost_percent = 0.0;
                          }
                      }else{
                          metrics.interval_packet_count = 0;
                          metrics.interval_cnt_error = 0;
                      }
                  }else{
                      metrics.bytesTransferred = 0;
                      metrics.bandwidth =  0;
                  }
              }
          }
      }
      currentMetrics.append(metrics);
    }
    // SIGNAL AT ONCE: Emit the entire batch of stream data to your UI/Main thread
    emit iperfIntervalReport(currentMetrics);
}

double Iperf3Worker::getUnitFormat(char funit, QString &unitfmt)
{
    //funit  => a: auto, k,K,m,M,g,G,t,T,
    // qDebug() << "getUnitFormat: " << QString(funit);
    if (funit=='a'){
        unitfmt = "Mbit/sec";
        return 1000000.0;
    } else if (funit=='k'){
        unitfmt = "Kbit/sec";
        return 1000.0;
    } else if (funit=='m'){
        unitfmt = "Mbit/sec";
        return 1000000.0;
    } else if (funit=='g'){
        unitfmt = "Gbit/sec";
        return 1000000000.0;
    } else if (funit=='t'){
        unitfmt = "Tbit/sec";
        return 1000000000000.0;
        // following calc may not correct??
    } else if (funit=='K'){
        unitfmt = "Kbyte/sec";
        return 8192.0;
    } else if (funit=='M'){
        unitfmt = "Mbyte/sec";
        return 8192000.0;
    } else if (funit=='G'){
        unitfmt = "Gbyte/sec";
        return 8192000000.0;
    } else if (funit=='T'){
        unitfmt = "Tbyte/sec";
        return 8192000000000.0;
    } else {
        unitfmt = "Mbit/sec";
        return 1000000.0;
    }
}
