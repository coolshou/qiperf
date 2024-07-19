#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QQueue>
#include <QFileInfo>
#include <QDebug>


class FileServer : public QObject
{
    Q_OBJECT
public:
    explicit FileServer(quint16 port, QObject *parent = nullptr);
    void setRootPath(QString pathname);
protected:

private slots:
    void acceptFileConnection();
    void updateFileProgress();
    void displayError(QAbstractSocket::SocketError socketError);
    void sendFile(QString filename);

signals:
private:
    QTcpServer *fileserver;
    QTcpSocket *filesocket;
    QString m_rootpath;
    qint64 m_chunkSize;
    quint64 totalBytes;
    quint64 bytesReceived;
    qint64 filenameSize;
    qint64 bytesWritten;
    qint64  bytestoWrite;
    QString m_filename;

    QFile *m_localFile;
    QByteArray inBlock;
    QByteArray outBlock;

};

#endif // FILESERVER_H
