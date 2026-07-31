#include "fileclient.h"

#include <QDataStream>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

FileClient::FileClient(quint16 port, QString targetaddress, QObject *parent)
    : QObject{parent}, m_currentFile(nullptr), m_chunkSize(64 * 1024)
    , m_port(port), m_targetaddress(targetaddress)
{
    m_debuglv =3;
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

    debug("FileClient connect to " + targetaddress + " port: " + QString::number(port));
    fileSocket->connectToHost(targetaddress, port);
}

void FileClient::enqueueFile(QString filename)
{
    debug("file to send(enqueueFile): " + filename);
    m_fileQueue.append(filename);
    if (fileSocket->state() == QTcpSocket::UnconnectedState){
        fileSocket->connectToHost(m_targetaddress, m_port);
    }
    if (fileSocket->state() == QTcpSocket::ConnectedState && !m_currentFile) {
        //If the client is connected and not currently transferring a file, it will start sending the next file in the queue
        sendNextFile();
    }else {
        debug("fileSocket->state(): " + QString::number((int)fileSocket->state())
              + "  (2=ConnectingState,3=ConnectedState)");
        if (m_currentFile){
            debug("enqueueFile m_currentFile: " + m_currentFile->fileName());
        }
    }
}

QString FileClient::getTargetAddress()
{
    return m_targetaddress;
}

void FileClient::debug(QString msg, int lv)
{
    if (lv <=m_debuglv){
        qDebug() << "[FileClient]" << msg;
    }
}

void FileClient::onSetDebugLv(int lv)
{
    debug("FileClient::onSetDebugLv:" + QString::number(lv), 0);
    m_debuglv = lv;
}

void FileClient::onConnected()
{
    // Start sending the first file if there's any in the queue
    if (!m_currentFile) {
        sendNextFile();
    }
}

void FileClient::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    if (!m_currentFile || !m_currentFile->isOpen()) {
        return;
    }
    // 當 Socket 寫入緩衝區中的未傳送資料低於 2 倍 Chunk 大小 (128KB) 時才持續填入資料
    // 這可以防止寫入速度快於網路傳送速度時造成的記憶體暴漲與排隊異常
    while (fileSocket->bytesToWrite() < m_chunkSize * 2 && !m_currentFile->atEnd()) {
        QByteArray buffer = m_currentFile->read(m_chunkSize);
        if (!buffer.isEmpty()) {
            fileSocket->write(buffer);
        }
    }
    // 只有當檔案已讀完，且 Socket 緩衝區中的資料也全部發送完畢時，才算真正完成傳輸
    if (m_currentFile->atEnd() && fileSocket->bytesToWrite() == 0) {
        QString filename = m_currentFile->fileName();
        m_currentFile->close();

        if (!m_currentFile->remove()){
            debug("Delete file fail:" + filename);
        }
        delete m_currentFile;
        m_currentFile = nullptr;

        debug("[onBytesWritten] File transfer completed: " + filename);
        sendNextFile(); // 繼續傳送佇列中的下一個檔案
    }
    // if (m_currentFile && m_currentFile->isOpen()) {
    //     QByteArray buffer = m_currentFile->read(m_chunkSize); // Read in chunks of 64KB
    //     if (buffer.isEmpty()) {
    //         //no more data to send
    //         QString filename = m_currentFile->fileName();
    //         m_currentFile->close();
    //         //delete file which had finished sending
    //         if (!m_currentFile->remove()){
    //             debug("Delete file fail:" + filename);
    //         }
    //         delete m_currentFile;
    //         m_currentFile = nullptr;
    //         debug("[onBytesWritten]File transfer completed: " + filename);
    //         sendNextFile(); // Proceed to the next file in the queue
    //     } else {
    //         fileSocket->write(buffer);
    //     }
    // }
}

void FileClient::onDisconnected()
{
    debug("FileClient Disconnected from server", 5);
    if (m_currentFile) {
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
    }
}

void FileClient::sendNextFile()
{
    if (m_fileQueue.isEmpty()) {
        debug("No Queue file");
        return;
    }
    QString filePath = m_fileQueue.dequeue();
    m_currentFile = new QFile(filePath);
    if (!m_currentFile->open(QIODevice::ReadOnly)) {
        debug("Cannot open file" + filePath + ":" + m_currentFile->errorString());
        delete m_currentFile;
        m_currentFile = nullptr;
        sendNextFile(); // Try to send the next file
        return;
    }
    QFileInfo fileInfo(*m_currentFile);
    QString header = QString("FILE:%1:%2\n").arg(fileInfo.fileName()).arg(fileInfo.size());
    fileSocket->write(header.toUtf8());
    // 主動觸發第一次讀取，啟動傳輸迴圈
    onBytesWritten(0);
}
