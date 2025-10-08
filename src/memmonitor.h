#ifndef MEMMONITOR_H
#define MEMMONITOR_H

#pragma once

#include <QObject>
#include <QTimer>

class MemMonitor : public QObject {
    Q_OBJECT

public:
    explicit MemMonitor(int interval=1, QObject *parent = nullptr);
    ~MemMonitor();
    void start(int intervalMs = 1000); // default: 1 second
    void stop();
    void setInterval(int interval);
signals:
    void memoryUsageUpdated(qint64 memoryMB); // emitted periodically

private slots:
    void updateMemoryUsage();

private:
    QTimer *timer;
    qint64 getMemoryUsageKB();
    int mInterval; //ms
};

#endif // MEMMONITOR_H
