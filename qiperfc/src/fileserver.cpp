#include "fileserver.h"

#include <QDataStream>
#include <QDir>

#include <QDebug>


FileServer::FileServer(quint16 port, QObject *parent)
    : QObject{parent}, m_chunkSize(64 * 1024)
{
//    filesocket = new QTcpSocket(this);
    fileserver = new QTcpServer(this);
    fileserver->listen(QHostAddress::AnyIPv4, port);
    qDebug() << "FileServer listen on: " << port;
    connect(fileserver, &QTcpServer::newConnection, this, &FileServer::acceptFileConnection);
    // connect(fileserver, &QTcpServer:, this, SLOT(acceptFileConnection()));
    bytesReceived = 0;
    totalBytes = 0;
    filenameSize = 0;
}

void FileServer::setRootPath(QString pathname)
{
    QDir dir(pathname);
    if (!dir.exists()){
        if (!dir.mkpath(".")) {
            qCritical() << "Cannot create output directory:" << pathname;
            return;
        }
    }
    m_rootpath = pathname;
    //update all FileSaveSocket's m_rootpath
    foreach(auto *fsock, m_filesocks){
        fsock->setRootpath(m_rootpath);
    }
}

void FileServer::close()
{
    m_filesocks.clear();
}

int FileServer::getSockets()
{
    return m_filesocks.length();
}

void FileServer::acceptFileConnection()
{
    bytesWritten = 0;

    QTcpSocket *filesocket = fileserver->nextPendingConnection();
//    qDebug() << "acceptFileConnection " << filesocket << " (" << filesocket->peerAddress().toString() << ") with rootpath: " << m_rootpath;
    FileSaveSocket *customSocket = new FileSaveSocket(m_rootpath, filesocket);
    m_filesocks.append(customSocket);
//    m_filesocks.insert(filesocket->peerAddress().toString(),customSocket);
    connect(customSocket, &FileSaveSocket::finished,this, &FileServer::onFinished);
}
void FileServer::slotReceive(QTcpSocket* socket)
{
    Q_UNUSED(socket);
}

void FileServer::onFinished()
{
    FileSaveSocket *socket = qobject_cast<FileSaveSocket *>(sender());
    if (socket) {
        // qDebug() << "FileServer::onFinished():";// << socket->peerAddress().toString();
        if(m_filesocks.contains(socket)){
            int rc = m_filesocks.removeAll(socket);
            qDebug() << "FileServer::onFinished():removeAll: " << rc;
        }
    }
}

void FileServer::sendFile(QString filename)
{
    m_localFile = new QFile(filename);
    if (!m_localFile->open(QFile::ReadOnly))
    {
        qDebug()<< "open file error: " << filename;
        return;
    }

    totalBytes = m_localFile->size();
    QDataStream sendout(&outBlock, QIODevice::WriteOnly);
    sendout.setVersion(QDataStream::Qt_5_15);
    QString currentFileName = filename.right(filename.size() - filename.lastIndexOf('/') - 1);
    qDebug() << "sendFile: " << filename;

    sendout << qint64(0) << qint64(0) << currentFileName;
    totalBytes += outBlock.size();
    sendout.device()->seek(0);
    sendout << totalBytes << qint64((outBlock.size() - sizeof(qint64)* 2));

    qDebug() << "TODO: send to which socket???";
    //bytestoWrite = totalBytes - filesocket->write(outBlock);
    outBlock.resize(0);
}
