#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>

class FileClient : public QObject
{
    Q_OBJECT
public:
    explicit FileClient(quint16 port, QString targetaddress="127.0.0.1", QObject *parent = nullptr);
    void initTCP(quint16 port, QString targetaddress);
    void sendFile(QString filename);

signals:
private slots:
//    void connectServer();
//    void receiveData();
    void updateFileProgress(qint64 numBytes);
    void updateFileProgress();

private:
//    QTcpSocket *tcpSocket;  // for text message exchange
    QTcpSocket *fileSocket;  //for file transfer

    qint64 m_chunkSize;
    qint64 m_port;
    QString m_targetaddress;

    quint64 totalBytes;
    quint64 bytesWritten;
    qint64 bytestoWrite;
    qint64 filenameSize;
    quint64 bytesReceived;

    QString m_filename;

    QFile *m_localFile;
    QByteArray outBlock;
    QByteArray inBlock;
};

#endif // FILECLIENT_H
