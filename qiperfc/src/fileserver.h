#ifndef FILESERVER_H
#define FILESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QQueue>
#include <QFileInfo>
#include <QDebug>

#include "filesaveSocket.h"

class FileServer : public QObject
{
    Q_OBJECT
public:
    explicit FileServer(quint16 port, QObject *parent = nullptr);
    void setRootPath(QString pathname);
    void close();
    int getSockets();

public slots:
    void slotReceive(QTcpSocket *socket);
    void onFinished();
protected:

private slots:
    void acceptFileConnection();
//    void updateFileProgress();
//    void displayError(QAbstractSocket::SocketError socketError);
    void sendFile(QString filename);

signals:
private:
    QTcpServer *fileserver;
    QString m_rootpath;
    qint64 m_chunkSize;
    quint64 bytesReceived;
    qint64 filenameSize;
    quint64 bytesWritten;
    qint64  bytestoWrite;
    QString m_filename;
    QList<FileSaveSocket *> m_filesocks; // list of FileSaveSocket
//    QMap<QString, FileSaveSocket *> m_filesocks; // list of FileSaveSocket
    QFile *m_localFile;
//    QByteArray inBlock;
    quint64 totalBytes;
    QByteArray outBlock;

};

#endif // FILESERVER_H
