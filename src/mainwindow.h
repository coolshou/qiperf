#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QProcess>

#include "iperfworker.h"

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
    void on_pb_run_clicked();
    void readStdOut(QString text);
    void readStdErr(QString text);
    void onStarted();
    void onFinished(int exitCode, int exitStatus);
    void onLog(QString text);

    void on_pb_clear_clicked();

    void on_pb_quit_clicked();

    void on_pb_copy_clicked();

private:
#if defined (Q_OS_ANDROID)
    QString m_path;
    QString m_iperfexe2;
    QString m_iperfexe3;
#endif

    Ui::MainWindow *ui;
    QThread *iperf_th;
    IperfWorker *iperfer;

};
#endif // MAINWINDOW_H
