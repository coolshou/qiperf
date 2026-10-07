#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QProcess>

#include "iperfworker.h"
#include "iperf3wrapper.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStart(bool checked);
    void onStop(bool checked);

    void readStdOut(QString text);
    void readStdErr(QString text);
    void onStarted();
    void onFinished(int exitCode, int exitStatus);

    void on_pb_clear_clicked();
    void on_pb_quit_clicked();
    void on_pb_copy_clicked();
    //
    void onTestFinished(const QString &jsonResult);
    void onTestError(const QString& errorMessage);
    void onTestStarted(quintptr threadId);
    void onLog(const QString& msg);
    void onIperfIntervalReport(const QList<StreamMetrics> &streamList);
private:


    Ui::MainWindow *ui;
    QThread *iperf_th;
    IperfWorker *iperfer;
    QString serverHost;
    int serverPort = 5201;
    int duration = 30;
    int streams = 1;
    bool isClient= false;
    Iperf3Wrapper *wrapper;
    quintptr mThreadId;

};
#endif // MAINWINDOW_H
