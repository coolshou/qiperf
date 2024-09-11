#ifndef FILESAVESOCKET_H
#define FILESAVESOCKET_H

#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include <QDir>

class FileSaveSocket: public QObject
{
    Q_OBJECT
public:
        FileSaveSocket(QString pathname, QTcpSocket* socket);
        ~FileSaveSocket();
        QString getData();
        void write(QString data);
        void setRootpath(QString rootpath);

private slots:
        void onReadyRead();
        void onClientDisconnected();
signals:
        void dataReady(QTcpSocket *socket);
        void socketConnected(FileSaveSocket *socket);
        void socketDisconnected(FileSaveSocket *socket);
        void finished(); //file transfer finish
private:
    QTcpSocket* tcpSocket;
    QString m_rootpath;
    QByteArray inBlock;
    quint64 totalBytes;
    qint64 bytesReceived;
    qint64 filenameSize;
    QString m_filename;
    QFile *m_localFile = nullptr;
    qint64 m_fileSize;

};
#endif
