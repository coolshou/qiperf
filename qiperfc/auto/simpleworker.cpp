#include "simpleworker.h"

#include <QApplication>
#include <QEventLoop>
#include <QThread>
#include <QDebug>

SimpleWorker::SimpleWorker(QStringList files, QString savepath, QObject *parent)
    : QObject{parent}, m_files(files), m_savepath(savepath)
{
    m_stop = false;
    m_skip = false;
    m_running = false;
    connect(this, &SimpleWorker::started, this, &SimpleWorker::onStarted);
    connect(this, &SimpleWorker::stoped, this, &SimpleWorker::onStoped);
}

void SimpleWorker::run()
{
    // this should run in thread!!
    emit started();
    int idx=0;
    for (const QString &filename : m_files) {
        // qDebug() << " let qiperfc load file " << filename;
        m_tpdata.insert(idx, -0.1);
        m_lostratedata.insert(idx, 0.0);
        emit loadfile(QString::number(idx), filename, m_savepath);
        // wait result
        while (m_tpdata.value(idx)==-0.1)
        {
            // qDebug() << "wait TP value:" << QString::number(m_tpdata.value(idx));
            QThread::sleep(1);
            QApplication::processEvents(QEventLoop::AllEvents);
            if (m_stop){
                break;
            }
            if (m_skip){
                m_skip = false;
                break;
            }
        }
        // qDebug() << "qiperfc start test";
        // qDebug() << "wait qiperfc info finish and update data";

        // repeat next file
        QThread::sleep(3);
        idx++;
        if (m_stop){
            break;
        }
        emit progress(idx);
        QApplication::processEvents(QEventLoop::AllEvents);
    }
    emit stoped();
}

void SimpleWorker::setStop()
{
    // TODO: force to stop
    m_stop = true;
}

bool SimpleWorker::isRunning()
{
    return m_running;
}

void SimpleWorker::setTPLostRate(int idx, double value, double lostrate)
{
    qDebug() << "SimpleWorker::setTPLostRate: idx:" << QString::number(idx) <<
        " value:" << QString::number(value);
    if (m_tpdata.contains(idx)){
        m_tpdata[idx] = value;
        m_lostratedata[idx] = lostrate;
    }else {
        qDebug() << "FIXME: we should have key:" << idx << " in m_tpdata:" << m_tpdata;
    }
}

void SimpleWorker::onStarted()
{
    m_stop = false;
    m_skip = false;
    m_running=true;
}

void SimpleWorker::onStoped()
{
    m_running=false;
}
