#include "fileclient.h"

#include <QDataStream>
#include <QFileInfo>

#include <QDebug>

FileClient::FileClient(quint16 port, QString targetaddress, QObject *parent)
    : QObject{parent}, m_currentFile(nullptr), m_chunkSize(64 * 1024)
    , m_port(port), m_targetaddress(targetaddress)
{
    initTCP(m_port, m_targetaddress);

    totalBytes = 0;
    bytesReceived = 0;
}

FileClient::~FileClient()
{
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
    }
    fileSocket->close();
}

void FileClient::initTCP(quint16 port, QString targetaddress)
{
    if (!targetaddress.contains(m_targetaddress)){
        m_targetaddress = targetaddress;
    }
    if (port !=m_port) {
        m_port = port;
    }
    fileSocket = new QTcpSocket(this);
    fileSocket->abort();
    connect(fileSocket, &QTcpSocket::connected, this, &FileClient::onConnected);
    connect(fileSocket, &QTcpSocket::disconnected, this, &FileClient::onDisconnected);
    connect(fileSocket, &QTcpSocket::bytesWritten, this, &FileClient::onBytesWritten);

    qDebug() << "FileClient connect to " << targetaddress << " port: " << port;
    fileSocket->connectToHost(targetaddress, port);


}

void FileClient::enqueueFile(QString filename)
{
    qInfo()<< "file to send(enqueueFile): " << filename;
    m_fileQueue.append(filename);
    if (fileSocket->state() == QTcpSocket::ConnectedState && !m_currentFile) {
        //If the client is connected and not currently transferring a file, it will start sending the next file in the queue
        sendNextFile();
    }else {
        qDebug() << "fileSocket->state(): " << (int)fileSocket->state() << "  (3=ConnectedState)" ;
        if (m_currentFile){
            qDebug() << "enqueueFile m_currentFile: " << m_currentFile->fileName();
        }
    }
}

QString FileClient::getTargetAddress()
{
    return m_targetaddress;
}

void FileClient::onConnected()
{
    qDebug() << "Connected to server";
    // Start sending the first file if there's any in the queue
    if (!m_currentFile) {
        sendNextFile();
    }
}

void FileClient::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    if (m_currentFile && m_currentFile->isOpen()) {
        QByteArray buffer = m_currentFile->read(m_chunkSize); // Read in chunks of 64KB
        if (buffer.isEmpty()) {
            QString filename = m_currentFile->fileName();
            m_currentFile->close();
            delete m_currentFile;
            m_currentFile = nullptr;
            qInfo() << "[onBytesWritten]File transfer completed: " << filename;
            sendNextFile(); // Proceed to the next file in the queue
        } else {
            fileSocket->write(buffer);
        }
    }
}

void FileClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
    }
}

void FileClient::sendNextFile()
{
    if (m_fileQueue.isEmpty()) {
        qInfo() << "All files have been sent";
        return;
    }

    QString filePath = m_fileQueue.dequeue();
    m_currentFile = new QFile(filePath);
    if (!m_currentFile->open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file" << filePath << ":" << m_currentFile->errorString();
        delete m_currentFile;
        m_currentFile = nullptr;
        sendNextFile(); // Try to send the next file
        return;
    }

    QFileInfo fileInfo(*m_currentFile);
    QString header = QString("FILE:%1:%2\n").arg(fileInfo.fileName()).arg(fileInfo.size());
//    qDebug() << "header: " << header;
    fileSocket->write(header.toUtf8());
}
