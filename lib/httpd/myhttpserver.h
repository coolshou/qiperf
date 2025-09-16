#ifndef MYHTTPSERVER_H
#define MYHTTPSERVER_H

#include <QtCore>
#include <QtNetwork>
#include <QObject>
#include <QString>

static QString urlDecode(const QByteArray &s) {
    return QUrl::fromPercentEncoding(s);
}

static QString contentTypeForFile(const QString &fileName) {
    const QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext == "html" || ext == "htm") return "text/html; charset=utf-8";
    if (ext == "txt" || ext == "log" || ext == "csv") return "text/plain; charset=utf-8";
    if (ext == "json") return "application/json";
    if (ext == "pdf") return "application/pdf";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "zip") return "application/zip";
    return "application/octet-stream";
}

class ServerConfig : public QObject {
    Q_OBJECT

public:
    explicit ServerConfig(QObject* parent = nullptr)
        : QObject(parent), rootDir("") {}

    QString getRootDir() const {
        QReadLocker locker(&lock);
        return rootDir;
    }

public slots:
    void setRootDir(const QString& newRootDir) {
        QWriteLocker locker(&lock);
        rootDir = newRootDir;
    }


private:
    QString rootDir;
    mutable QReadWriteLock lock;
};

class ConnectionHandler : public QObject {
    Q_OBJECT
public:
    ConnectionHandler(QTcpSocket *socket, ServerConfig *config, QObject *parent=nullptr)
        : QObject(parent), m_socket(socket), m_config(config) {
        m_socket->setParent(this);
        connect(m_socket, &QTcpSocket::readyRead, this, &ConnectionHandler::onReadyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, &ConnectionHandler::onDisconnected);
    }

private slots:
    void onReadyRead() {
        m_buffer += m_socket->readAll();
        // Simple request parsing: read until header end \r\n\r\n
        int headerEnd = m_buffer.indexOf("\r\n\r\n");
        if (headerEnd < 0) return;

        QByteArray header = m_buffer.left(headerEnd);
        // We only handle GET and ignore body (no POST support)
        QList<QByteArray> lines = header.split('\n');
        if (lines.isEmpty()) { sendSimple(400, "Bad Request"); return; }

        QByteArray requestLine = lines.first().trimmed();
        QList<QByteArray> parts = requestLine.split(' ');
        if (parts.size() < 3) { sendSimple(400, "Bad Request"); return; }

        QByteArray method = parts[0];
        QByteArray target = parts[1];
        QByteArray version = parts[2];

        if (method != "GET") { sendSimple(405, "Method Not Allowed", {{"Allow","GET"}}); return; }
        if (version != "HTTP/1.1" && version != "HTTP/1.0") { sendSimple(505, "HTTP Version Not Supported"); return; }

        // Parse path and query
        QUrl url(QString::fromUtf8(target));
        QString path = url.path();
        QUrlQuery query(url);

        if (path == "/" || path.isEmpty()) {
            handleRootList();
        } else {
            // Remove leading slash
            QString relPath = path.mid(1);
            relPath = QUrl::fromPercentEncoding(relPath.toUtf8());

            QUrlQuery q;
            q.addQueryItem("path", relPath);
            handleDownload(q);
        }
    }
    void handleRootList() {
        QString absRoot = rootDirSafe();
        QDir dir(absRoot);
        QFileInfoList entries = dir.entryInfoList(QDir::Files, QDir::Name);

        QString html;
        html += "<html><head><meta charset=\"utf-8\"><title>File List</title></head><body>";
        html += "<h3>Files in root</h3><ul>";

        for (const QFileInfo &fi : entries) {
            QString name = fi.fileName();
            QString encoded = QString::fromUtf8(QUrl::toPercentEncoding(name));
            html += "<li><a href=\"/" + encoded + "\">" + name + "</a></li>";
        }

        html += "</ul></body></html>";

        sendResponse(200, "OK", "text/html; charset=utf-8", html.toUtf8());
    }
    void onDisconnected() {
        m_socket->deleteLater();
        this->deleteLater();
    }

private:
    void sendIndex() {
        const QByteArray body =
            "<html><head><meta charset=\"utf-8\"><title>Qt File Server</title></head>"
            "<body>"
            "<h3>Qt File Server</h3>"
            "<p>Use /list?path=relative/subdir to list files.</p>"
            "<p>Use /download?path=relative/file to download a file.</p>"
            "</body></html>";
        sendResponse(200, "OK", "text/html; charset=utf-8", body);
    }

    // Ensure requested relative path stays within root
    bool resolveSafePath(const QString &relative, QString *outAbsolute, bool wantDir) {
        QString rel = relative;
        // Treat empty as "."
        if (rel.trimmed().isEmpty()) rel = ".";
        // Decode and normalize
        QString decoded = urlDecode(rel.toUtf8());
        QDir base(rootDirSafe());
        QString abs = base.absoluteFilePath(decoded);
        QString canonBase = QFileInfo(base.canonicalPath()).absoluteFilePath();
        QString canonAbs = QFileInfo(abs).canonicalFilePath();

        if (canonBase.isEmpty()) canonBase = QFileInfo(base.absolutePath()).absoluteFilePath();
        if (canonAbs.isEmpty()) {
            // Path doesn't exist; still ensure it's within base by path check
            canonAbs = QDir::cleanPath(abs);
        }

        // Check sandboxing
        if (!canonAbs.startsWith(canonBase)) return false;

        if (wantDir) {
            // For listing, allow non-existent? No — require existing dir
            QFileInfo fi(canonAbs);
            if (!fi.exists() || !fi.isDir()) return false;
        } else {
            // For files, ensure it exists and is file
            QFileInfo fi(canonAbs);
            if (!fi.exists() || !fi.isFile()) return false;
        }

        *outAbsolute = canonAbs;
        return true;
    }

    QString rootDirSafe() const {
        return m_config->getRootDir();
    }

    void handleList(const QUrlQuery &query) {
        QString rel = query.queryItemValue("path");
        QString abs;
        if (!resolveSafePath(rel, &abs, /*wantDir=*/true)) {
            sendSimple(400, "Invalid or forbidden path");
            return;
        }

        QDir dir(abs);
        QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries, QDir::DirsFirst | QDir::Name);

        QJsonArray arr;
        for (const QFileInfo &fi : entries) {
            QJsonObject obj;
            obj["name"] = fi.fileName();
            obj["type"] = fi.isDir() ? "dir" : "file";
            obj["size"] = fi.isDir() ? 0 : static_cast<qint64>(fi.size());
            obj["modified"] = fi.lastModified().toUTC().toString(Qt::ISODate);
            arr.append(obj);
        }
        QJsonObject root;
        root["path"] = QDir(rootDirSafe()).relativeFilePath(abs);
        root["items"] = arr;
        QByteArray body = QJsonDocument(root).toJson(QJsonDocument::Compact);
        sendResponse(200, "OK", "application/json", body);
    }

    void handleDownload(const QUrlQuery &query) {
        QString rel = query.queryItemValue("path");
        QString abs;
        if (!resolveSafePath(rel, &abs, /*wantDir=*/false)) {
            sendSimple(400, "Invalid or forbidden path");
            return;
        }

        QFile file(abs);
        if (!file.open(QIODevice::ReadOnly)) {
            sendSimple(404, "File not found");
            return;
        }

        const qint64 size = file.size();
        QByteArray header;
        header += "HTTP/1.1 200 OK\r\n";
        header += "Content-Type: " + contentTypeForFile(abs).toUtf8() + "\r\n";
        header += "Content-Length: " + QByteArray::number(size) + "\r\n";
        header += "Accept-Ranges: none\r\n";
        header += "Connection: close\r\n";
        header += "Content-Disposition: attachment; filename=\"" + QFileInfo(abs).fileName().toUtf8() + "\"\r\n";
        header += "\r\n";

        // Send header first
        if (m_socket->write(header) == -1) { m_socket->disconnectFromHost(); return; }

        // Stream file in chunks to avoid high memory usage
        static const qint64 CHUNK = 1 << 16; // 64 KiB
        while (!file.atEnd()) {
            if (!m_socket->isWritable()) break;
            QByteArray chunk = file.read(CHUNK);
            if (chunk.isEmpty()) break;
            qint64 written = m_socket->write(chunk);
            if (written == -1) break;
            // Optional: throttle with waitForBytesWritten to backpressure
            if (!m_socket->waitForBytesWritten(30000)) break;
        }
        m_socket->disconnectFromHost();
    }

    void sendSimple(int code, const QByteArray &message, const QMap<QByteArray, QByteArray> &extraHeaders = {}) {
        QByteArray body = message + "\n";
        QByteArray hdr;
        hdr += "HTTP/1.1 " + QByteArray::number(code) + " " + message + "\r\n";
        hdr += "Content-Type: text/plain; charset=utf-8\r\n";
        hdr += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
        hdr += "Connection: close\r\n";
        for (auto it = extraHeaders.constBegin(); it != extraHeaders.constEnd(); ++it) {
            hdr += it.key() + ": " + it.value() + "\r\n";
        }
        hdr += "\r\n";
        m_socket->write(hdr);
        m_socket->write(body);
        m_socket->disconnectFromHost();
    }

    void sendResponse(int code, const QByteArray &status, const QByteArray &contentType, const QByteArray &body) {
        QByteArray hdr;
        hdr += "HTTP/1.1 " + QByteArray::number(code) + " " + status + "\r\n";
        hdr += "Content-Type: " + contentType + "\r\n";
        hdr += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
        hdr += "Connection: close\r\n";
        hdr += "\r\n";
        m_socket->write(hdr);
        m_socket->write(body);
        m_socket->disconnectFromHost();
    }

private:
    QTcpSocket *m_socket;
    ServerConfig *m_config;
    QByteArray m_buffer;
};

class MyHttpServer : public QTcpServer {
    Q_OBJECT
public:
    MyHttpServer(ServerConfig *cfg, QObject *parent=nullptr) : QTcpServer(parent), m_config(std::move(cfg)) {}
public slots:
    void start(quint16 port, QString rootpath, QHostAddress host=QHostAddress::Any){
        m_config->setRootDir(rootpath);
        if (!listen(host, port)) {
            qCritical() << "Failed to listen on port" << port << ":" << this->errorString();
            return;
        }
        emit started();
    }
    void stop(){
        if (isListening()){
            close();
        }
        emit stoped();
    }
signals:
    void started();
    void stoped();
protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *socket = new QTcpSocket;
        if (!socket->setSocketDescriptor(socketDescriptor)) {
            delete socket;
            return;
        }

        // Per-connection thread
        QThread *thread = new QThread;
        socket->moveToThread(thread);

        auto *handler = new ConnectionHandler(socket, m_config);
        handler->moveToThread(thread);

        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        connect(socket, &QTcpSocket::disconnected, thread, &QThread::quit);

        thread->start();
    }

private:
    ServerConfig *m_config;
};

// int main(int argc, char *argv[]) {
//     QCoreApplication app(argc, argv);

//     QCommandLineParser parser;
//     parser.setApplicationDescription("Qt Threaded File HTTP Server");
//     parser.addHelpOption();
//     QCommandLineOption portOpt({"p","port"}, "Port to listen on", "port", "8080");
//     QCommandLineOption rootOpt({"r","root"}, "Root directory to serve", "path", QDir::currentPath());
//     parser.addOption(portOpt);
//     parser.addOption(rootOpt);
//     parser.process(app);

//     bool ok = false;
//     quint16 port = parser.value(portOpt).toUShort(&ok);
//     if (!ok || port == 0) port = 8080;

//     QString rootDir = QDir(parser.value(rootOpt)).absolutePath();

//     auto config = QSharedPointer<ServerConfig>::create();
//     {
//         QWriteLocker locker(&config->lock);
//         config->rootDir = rootDir;
//     }

//     HttpServer server(config);
//     if (!server.listen(QHostAddress::Any, port)) {
//         qCritical() << "Failed to listen on port" << port << ":" << server.errorString();
//         return 1;
//     }

//     qInfo() << "Serving" << rootDir << "on port" << port;
//     return app.exec();
// }

#endif // MYHTTPSERVER_H
