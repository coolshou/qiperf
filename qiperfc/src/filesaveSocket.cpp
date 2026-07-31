#include "filesaveSocket.h"

#include <QHostAddress>
#include <QDebug>
#include <QCoreApplication>
#include <QEventLoop>

// read tcpSocket data and save to file
// FILE:<filename>:<filesize>

FileSaveSocket::FileSaveSocket (QString pathname, QTcpSocket* socket)
{
    setRootpath(pathname);
    tcpSocket = socket;
    connect(tcpSocket, &QTcpSocket::readyRead, this, &FileSaveSocket::onReadyRead);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &FileSaveSocket::onClientDisconnected);

}

FileSaveSocket::~FileSaveSocket ()
{
    //delete tcpSocket;
}

void FileSaveSocket::onReadyRead()
{
    if (!tcpSocket) return;
    // 使用迴圈持續處理，直到 Socket 資料不足以滿足目前狀態的需求
    while (tcpSocket->bytesAvailable() > 0) {
        // 狀態 1：還沒有建立檔案，準備讀取 Header
        if (m_localFile == nullptr) {
            // 關鍵：必須確保已經收到包含 '\n' 的完整一行 Header
            if (!tcpSocket->canReadLine()) {
                // 資料還不夠構成完整的一行標頭，等待下一次 onReadyRead 觸發
                return;
            }
            QByteArray line = tcpSocket->readLine();
            if (line.startsWith("FILE:")) {
                qDebug() << "prepare to receive file: " << line;
                QStringList headerParts = QString(line).trimmed().split(':');
                if (headerParts.size() == 3) {
                    m_filename = headerParts[1];
                    m_fileSize = headerParts[2].toLongLong();
                    QString filePath = m_rootpath + QDir::separator() + m_filename;
                    // qDebug() << "FileSaveSocket::onReadyRead NEW file: " << filePath;
                    bytesReceived = 0;
                    m_localFile = new QFile(filePath);
                    if (!m_localFile->open(QIODevice::WriteOnly)) {
                        qWarning() << "Cannot open file for writing " << filePath << ":" << m_localFile->errorString();
                        delete m_localFile;
                        m_localFile = nullptr;
                        return;
                    }
                }else{
                    qDebug() << "Wrong format of file('FILE:<filename>:<filesize>'):" << line;
                }
            }
        }
        // 狀態 2：檔案已開啟，開始接收檔案內容 (Body)
        if (m_localFile != nullptr) {
            // 計算目前還剩多少位元組沒接收
            qint64 bytesToRead = m_fileSize - bytesReceived;
            // 每次只讀取「Socket 現有資料」與「剩餘檔案大小」的較小值，避免吃掉下一個檔案的 Header
            QByteArray buffer = tcpSocket->read(qMin(bytesToRead, tcpSocket->bytesAvailable()));
            // QByteArray buffer = tcpSocket->read(qMin(m_fileSize - bytesReceived, (qint64)tcpSocket->bytesAvailable()));
            // bytesReceived += buffer.size();
            if (!buffer.isEmpty()) {
                m_localFile->write(buffer);
                bytesReceived += buffer.size();
            }

            // 檔案接收完畢處理
            if (bytesReceived == m_fileSize) {
                qDebug() << "FileSaveSocket::onReadyRead File finish received:" << m_localFile->fileName();
                m_localFile->flush(); // 確保資料寫入實體磁碟
                m_localFile->close();
                delete m_localFile;
                m_localFile = nullptr;
                bytesReceived = 0;
                m_fileSize = 0;
                emit finished();
            }
        }
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
}

void FileSaveSocket::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        clientSocket->deleteLater();
        emit socketDisconnected(this);
    }
}

QString FileSaveSocket::getData()
{
    qDebug()<< "FileSaveSocket::getData: canReadLine: " << tcpSocket->canReadLine();
    QByteArray ba = tcpSocket->readLine();
    qDebug() << ba;
    return QString(ba);
}

void FileSaveSocket :: write(QString data)
{
    tcpSocket->write(data.toStdString().c_str());
}

void FileSaveSocket::setRootpath(QString rootpath)
{
    // qDebug() << "FileSaveSocket::setRootpath: "  << rootpath;
    m_rootpath = rootpath;
}
