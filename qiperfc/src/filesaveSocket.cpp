#include "filesaveSocket.h"

#include <QHostAddress>
#include <QDebug>

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
//    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) return;
    while (tcpSocket->bytesAvailable() > 0) {
        if (m_localFile == nullptr) {
            QByteArray line = tcpSocket->readLine();
            if (line.startsWith("FILE:")) {
                QStringList headerParts = QString(line).trimmed().split(':');
                if (headerParts.size() == 3) {
                    m_filename = headerParts[1];
                    m_fileSize = headerParts[2].toLongLong();
                    QString filePath = m_rootpath + QDir::separator() + m_filename;
                    bytesReceived = 0;
                    m_localFile = new QFile(filePath);
                    if (!m_localFile->open(QIODevice::WriteOnly)) {
                        qWarning() << "Cannot open file for writing " << filePath << ":" << m_localFile->errorString();
                        delete m_localFile;
                        m_localFile = nullptr;
                        return;
                    }
                }
            }
        } else {
            QByteArray buffer = tcpSocket->read(qMin(m_fileSize - bytesReceived, (qint64)tcpSocket->bytesAvailable()));
            bytesReceived += buffer.size();

            m_localFile->write(buffer);
            if (bytesReceived == m_fileSize) {
                qInfo() << "FileSaveSocket::onReadyRead File received:" << m_localFile->fileName();
                m_localFile->close();
                delete m_localFile;
                m_localFile = nullptr;

            }
        }
    }
}

void FileSaveSocket::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        qInfo() << "Client disconnected: " << clientSocket->peerAddress().toString();
        clientSocket->deleteLater();
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
//    qDebug() << "FileSaveSocket::setRootpath: "  << rootpath;
    m_rootpath = rootpath;
}
