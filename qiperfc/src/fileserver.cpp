#include "fileserver.h"

#include <QDataStream>
#include <QDir>
#include <QCoreApplication>
#include <QEventLoop>

#include <QDebug>


FileServer::FileServer(quint16 port, QObject *parent)
    : QObject{parent}, m_chunkSize(64 * 1024)
{
//    filesocket = new QTcpSocket(this);
    fileserver = new QTcpServer(this);
    if (!fileserver->listen(QHostAddress::AnyIPv4, port)){
        QString err = "FileServer:FileServer listen on port: " + QString::number(port) + " Fail!!";
        emit error(err);
    }else{
        qInfo() << "FileServer listen on: " << port;
    }
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
    m_rootpath = QDir::toNativeSeparators(pathname);
    // qDebug()  << "FileServer::setRootPath: m_filesocks.count: " << m_filesocks.count();
    //update all FileSaveSocket's m_rootpath
    // foreach(auto *fsock, m_filesocks){
    //     fsock->setRootpath(m_rootpath);
    //     QCoreApplication::processEvents(QEventLoop::AllEvents);
    // }
}

void FileServer::close()
{
    foreach (auto fs, m_filesocks) {
        fs->close();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    m_filesocks.clear();
}

qint64 FileServer::getSockets()
{
    return m_filesocks.length();
}

void FileServer::acceptFileConnection()
{
    bytesWritten = 0;

    QTcpSocket *filesocket = fileserver->nextPendingConnection();
    if (!m_filesocks.contains(filesocket)){
        connect(filesocket, &QTcpSocket::disconnected, this, &FileServer::onClientDisconnected);
        // qDebug() << "acceptFileConnection " << filesocket << " (" << filesocket->peerAddress().toString() << ") with rootpath: " << m_rootpath;
        FileSaveSocket *customSocket = new FileSaveSocket(m_rootpath, filesocket);
        // qDebug() << "current m_filesocks count: " << m_filesocks.count();
        m_filesocks.append(filesocket);
        connect(customSocket, &FileSaveSocket::finished,this, &FileServer::onFinished);
    }
}
void FileServer::slotReceive(QTcpSocket* socket)
{
    Q_UNUSED(socket);
}

void FileServer::onFinished()
{
    // FileSaveSocket *socket = qobject_cast<FileSaveSocket *>(sender());
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
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
    sendout.setVersion(QDataStream::Qt_5_11);
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

// void FileServer::onClientDisconnected(FileSaveSocket *socket)
void FileServer::onClientDisconnected()
{
    // client has disconnected, so remove from list
    QTcpSocket *pClient = static_cast<QTcpSocket *>(QObject::sender());
    // int rc= m_filesocks.removeAll(socket);
    int rc= m_filesocks.removeAll(pClient);
    if (rc >1){
        qDebug() << "FileServer::onClientDisconnected:remove more the one socket:" << rc << " !!!!";
    }
}
