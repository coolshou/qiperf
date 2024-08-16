#include "filewatcher.h"

#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QEventLoop>
#include <QDebug>

FileWatcher::FileWatcher(const QString &filePath, QObject *parent)
    : QObject{parent}, m_filePath(filePath)
{
    // Set up file watcher
    m_fileWatcher.addPath(filePath);
    // Connect signals
    connect(&m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &FileWatcher::onFileChanged);
    // Initial read
    readFile();
}

void FileWatcher::onFileChanged(const QString &path)
{
    // Delay the read to ensure the file has finished writing
    QTimer::singleShot(100, this, &FileWatcher::readFile);
    // Re-add the file to the watcher in case it was deleted and recreated
    m_fileWatcher.addPath(path);
}

void FileWatcher::readFile()
{
    QFile file(m_filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // DO NOT ADD any qDebug() in here, it will loop output to file
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            emit onNewLine(line);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    } else {
        QString s = "Failed to open file for reading:" + m_filePath;
        qWarning() << s;
        emit onNewLine(s);
    }
}
