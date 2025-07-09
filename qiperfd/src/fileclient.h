#ifndef FILECLIENT_H
#define FILECLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QQueue>

class FileClient : public QObject
{
    Q_OBJECT
public:
    explicit FileClient(quint16 port, QString targetaddress="127.0.0.1", QObject *parent = nullptr);
    ~FileClient();
    void initTCP(quint16 port, QString targetaddress);
    void enqueueFile(QString filename);
    QString getTargetAddress();
    void debug(QString msg, int lv=3);
public slots:
    void onSetDebugLv(int lv);

signals:
private slots:
    void onConnected();
    void onBytesWritten(qint64 bytes);
    void onDisconnected();

private:
    void sendNextFile();
    int m_debuglv;
    QTcpSocket *fileSocket;  //for file transfer
    QQueue<QString> m_fileQueue;
    QFile *m_currentFile;
    qint64 m_chunkSize;
    qint64 m_port;
    QString m_targetaddress;

    quint64 totalBytes;
    quint64 bytesReceived;
};

#endif // FILECLIENT_H
