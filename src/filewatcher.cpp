#include "filewatcher.h"

#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QEventLoop>

#include <stdio.h>

FileWatcher::FileWatcher(const QString &filePath, QObject *parent)
    : QObject{parent}, m_filePath(filePath)
{
    m_filepos = 0;
    m_fileWatcher = new QFileSystemWatcher();
    // Set up file watcher
    if(!m_fileWatcher->addPath(filePath)){
        // qDebug() <<"FileWatcher FAIL: " <<filePath;
        log("FileWatcher FAIL: " + filePath);
    }

    // Connect signals
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &FileWatcher::onFileChanged);
    // Initial read
    //QTimer::singleShot(3000, this, &FileWatcher::readFile); // slow??
    QTimer::singleShot(500, this, &FileWatcher::readFile);
}

void FileWatcher::onFileChanged(const QString &path)
{
    // Delay the read to ensure the file has finished writing
    QTimer::singleShot(100, this, &FileWatcher::readFile);
    // Re-add the file to the watcher in case it was deleted and recreated
    if (!m_fileWatcher->files().contains(path)){
        if (!m_fileWatcher->addPath(path)){
            // qDebug() <<
            QString  msg = "onFileChanged addPath FAIL: " + path;
            log(msg);
        }
    }

}

void FileWatcher::readFile()
{
    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // DO NOT ADD any qDebug() in here, it will loop output to file
        QTextStream in(&file);
        while (!in.atEnd()) {
            in.seek(m_filepos);
            QString line = in.readLine();
            emit onNewLine(line);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
            m_filepos = in.pos();
            //log("m_filepos: "+ QString::number(m_filepos));
        }
    } else {
        QString s = "Failed to open file for reading:" + m_filePath;
        // qWarning() << s;
        log(s);
        emit onNewLine(s);
    }
}

void FileWatcher::log(QString message)
{
    printf("%s\n", message.toStdString().c_str());
    fflush(stdout);
}
