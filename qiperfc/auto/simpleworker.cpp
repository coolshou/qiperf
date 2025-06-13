#include "simpleworker.h"

#include <QThread>
#include <QDebug>
SimpleWorker::SimpleWorker(QStringList files,QObject *parent)
    : QObject{parent}, m_files(files)
{}

void SimpleWorker::run()
{
    // this should run in thread!!
    int idx=0;
    for (const QString &filename : m_files) {
        qDebug() << " let qiperfc load file " << filename;
        emit loadfile(QString::number(idx), filename);
        qDebug() << "qiperfc start test";
        qDebug() << "wait qiperfc info finish and update data";

        //      repeat next file
        QThread::sleep(10);
        idx++;
    }
}
