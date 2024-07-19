#include "fileclient.h"

#include <QDataStream>

#include <QDebug>

FileClient::FileClient(quint16 port, QString targetaddress, QObject *parent)
    : QObject{parent}, m_chunkSize(64 * 1024)
    , m_port(port), m_targetaddress(targetaddress)
{
    initTCP(m_port, m_targetaddress);

    totalBytes = 0;
    bytestoWrite = 0;
    bytesWritten = 0;
    bytesReceived = 0;
    filenameSize = 0;
}

void FileClient::initTCP(quint16 port, QString targetaddress)
{
    fileSocket = new QTcpSocket(this);
    fileSocket->abort();
    fileSocket->connectToHost(targetaddress, port);
    connect(fileSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(updateFileProgress(qint64)));
    connect(fileSocket, SIGNAL(readyRead()), this, SLOT(updateFileProgress()));

}

void FileClient::sendFile(QString filename)
{
    m_localFile = new QFile(filename);
    if (!m_localFile->open(QFile::ReadOnly))
    {
        qDebug() << tr("Client:open file error!");
        return;
    }
    totalBytes = m_localFile->size();
    QDataStream sendout(&outBlock, QIODevice::WriteOnly);
    sendout.setVersion(QDataStream::Qt_5_15);
    QString currentFileName = filename.right(filename.size() - filename.lastIndexOf('/') - 1);

    qDebug() << sizeof(currentFileName);
        sendout << qint64(0) << qint64(0) << currentFileName;
    totalBytes += outBlock.size();
    sendout.device()->seek(0);
    sendout << totalBytes << qint64((outBlock.size() - sizeof(qint64)* 2));

    bytestoWrite = totalBytes - fileSocket->write(outBlock);
    outBlock.resize(0);
}

void FileClient::updateFileProgress(qint64 numBytes)
{
     bytesWritten += numBytes;

    if (bytestoWrite > 0)
    {
        outBlock = m_localFile->read(qMin(bytestoWrite, m_chunkSize));
        bytestoWrite -= (fileSocket->write(outBlock));
        outBlock.resize(0);
    }
    else
        m_localFile->close();

    qDebug() << "send/Total: " << bytesWritten << "/" << totalBytes;

    if (bytesWritten == totalBytes)
    {
        m_localFile->close();
        //fileSocket->close();
    }
}

void FileClient::updateFileProgress()
{
    QDataStream inFile(fileSocket);
    inFile.setVersion(QDataStream::Qt_5_15);

    if (bytesReceived <= sizeof(qint64)* 2)
    {
        if ((fileSocket->bytesAvailable() >= (qint64)(sizeof(qint64)) * 2) && (filenameSize == 0))
        {
            inFile >> totalBytes >> filenameSize;
            bytesReceived += sizeof(qint64)* 2;
        }
        if ((fileSocket->bytesAvailable() >= filenameSize) && (filenameSize != 0))
        {
            inFile >> m_filename;
            bytesReceived += filenameSize;
            m_localFile = new QFile(m_filename);
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
        bytesReceived += fileSocket->bytesAvailable();
        inBlock = fileSocket->readAll();
        m_localFile->write(inBlock);
        inBlock.resize(0);
    }
    if (bytesReceived == totalBytes)
    {
        qDebug() << "Receive file successfully! " << m_localFile->fileName();
        bytesReceived = 0;
        totalBytes = 0;
        filenameSize = 0;
        m_localFile->close();
        //fileSocket->close();
    }
}
