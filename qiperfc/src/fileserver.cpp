#include "fileserver.h"

#include <QDataStream>
#include <QDir>

FileServer::FileServer(quint16 port, QObject *parent)
    : QObject{parent}, m_chunkSize(64 * 1024)
{
    filesocket = new QTcpSocket(this);
    fileserver = new QTcpServer(this);
    fileserver->listen(QHostAddress::Any, port);
    connect(fileserver, SIGNAL(newConnection()), this, SLOT(acceptFileConnection()));
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
}

void FileServer::acceptFileConnection()
{
    bytesWritten = 0;

    filesocket = fileserver->nextPendingConnection();
    connect(filesocket, SIGNAL(readyRead()), this, SLOT(updateFileProgress()));
    connect(filesocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(updateFileProgress(qint64)));
    connect(filesocket, SIGNAL(bytesWritten(qint64)), this, SLOT(displayError(QAbstractSocket::SocketError socketError)));
}

void FileServer::updateFileProgress()
{
    QDataStream inFile(filesocket);
    inFile.setVersion(QDataStream::Qt_5_15);

    if (bytesReceived <= sizeof(qint64)* 2)
    {
        if ((filesocket->bytesAvailable() >= (qint64)(sizeof(qint64)) * 2) && (filenameSize == 0))
        {
            inFile >> totalBytes >> filenameSize;
            bytesReceived += sizeof(qint64)* 2;
        }
        if ((filesocket->bytesAvailable() >= filenameSize) && (filenameSize != 0))
        {
            inFile >> m_filename;
            bytesReceived += filenameSize;
            m_localFile = new QFile(m_rootpath + QDir::separator() + m_filename);
            if (!m_localFile->open(QFile::WriteOnly))
            {
                qDebug() << "Server::open file error!";
                return;
            }
        }
        else
            return;
    }
    if (bytesReceived < totalBytes)
    {
        bytesReceived += filesocket->bytesAvailable();
        inBlock = filesocket->readAll();
        m_localFile->write(inBlock);
        inBlock.resize(0);
    }

    qDebug() << "Received/Total: " << bytesReceived << "/" << totalBytes;
    if (bytesReceived == totalBytes)
    {
        qDebug() << "Receive file successfully!" << m_localFile->fileName();
        bytesReceived = 0;
        totalBytes = 0;
        filenameSize = 0;
        m_localFile->close();
        //filesocket->close();
    }
}

void FileServer::displayError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "displayError: " << socketError;
    qDebug() << filesocket->errorString();
    //    filesocket->close();
}

void FileServer::sendFile(QString filename)
{
    m_localFile = new QFile(filename);
    if (!m_localFile->open(QFile::ReadOnly))
    {
        qDebug()<< "open file error: " << filename;
        return;
    }

    this->totalBytes = m_localFile->size();
    QDataStream sendout(&outBlock, QIODevice::WriteOnly);
    sendout.setVersion(QDataStream::Qt_5_15);
    QString currentFileName = filename.right(filename.size() - filename.lastIndexOf('/') - 1);
    qDebug() << "sendFile: " << filename;

    sendout << qint64(0) << qint64(0) << currentFileName;
    totalBytes += outBlock.size();
    sendout.device()->seek(0);
    sendout << totalBytes << qint64((outBlock.size() - sizeof(qint64)* 2));

    bytestoWrite = totalBytes - filesocket->write(outBlock);
    outBlock.resize(0);
}
