/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "wsserver.h"
#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFileInfo>

#include "comm.h"

#include <QDebug>

QT_USE_NAMESPACE

//! [constructor]
WSServer::WSServer(quint16 port, QString mgr_ifname, MyInfo *myinfo, QObject *parent) :
    QObject(parent),m_port(port),m_ifname(mgr_ifname), m_myinfo(myinfo),
    m_pWebSocketServer(nullptr)
{
    /*
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WS Server"),
                                              QWebSocketServer::SecureMode,
                                              this);
    QSslConfiguration sslConfiguration;
    QFile certFile(QStringLiteral(":/ws.cert")); //TODO: create cert file automatic!!
    QFile keyFile(QStringLiteral(":/ws.key"));
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    m_pWebSocketServer->setSslConfiguration(sslConfiguration);
    */

    m_pWebSocketServer = new QWebSocketServer(QStringLiteral(QIPERFD_WSNAME),
                                              QWebSocketServer::NonSecureMode,
                                              this);
    connect(this , &WSServer::onUpdateInterface, this, &WSServer::updateListen);
    setIfname(mgr_ifname);

}
//! [constructor]

WSServer::~WSServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

QList<QString> WSServer::getClients()
{
    return m_clients.keys();
}

qint64 WSServer::sendTextMessage(QString msg, QString target)
{
    //send msg to target, if target is null, send to all connected client
    QList<QString> ts;
    if (!target.isNull()){
        ts.append(target);
    }else{
        ts = m_clients.keys();
    }
    qint64 rc=0;
    for(auto &t: as_const(ts)) {
        if (m_clients.contains(t)) {
//            onLog("TODO: send:" + msg + " back to " + t);
            m_sendtype=WSServer::sendtype::text;
//            qDebug() << "[sendTextMessage]TO: " << t <<" : " << msg;
            rc= m_clients.value(t)->sendTextMessage(msg);
            if (rc<=0){
                qDebug() << "ERROR sendText to " << t << " size=" << rc << " : " << msg;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return rc;

}

qint64 WSServer::sendBinaryMessage(QByteArray &data, QString target)
{
    QList<QString> ts;
    if (!target.isNull()){
        ts.append(target);
    }else{
        ts = m_clients.keys();
    }
    qint64 rc=0;
    for(auto &t: as_const(ts)) {
        if (m_clients.contains(t)) {
            rc = m_clients.value(t)->sendBinaryMessage(data);
            if (rc<=0){
                qDebug() << "ERROR sendBinaryMessage to " << t << " size=" << rc << " : " << data;
            }
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return rc;
}

void WSServer::onLog(QString text)
{
    qInfo() << "WSServer::onLog: " << text;
}

void WSServer::addFileToSend(QString filename, QString target)
{
    if (m_filenames.contains(filename)){
        qDebug() << "addFileToSend: File exist: " << filename;
        return;
    }
    m_filenames.append(filename);
    QFile *file = new QFile(filename);
    if (!file->open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file" << filename << ":" << file->errorString();
        delete file;
//        continue;
    }
    qDebug() << "send file: " << filename << " TO: " << m_currentClient;
    // qDebug() << "send file TO: " << m_currentClient;
    m_files.enqueue(file);
    m_sendtype=WSServer::sendtype::file;
    sendNextChunk(m_currentClient);

}

//! [onNewConnection]
void WSServer::onNewConnection()
{
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    QString sfrom = pSocket->peerAddress().toString();
    QString sfromPort = QString::number(pSocket->peerPort());
    QString speer= QString("%1:%2").arg(sfrom, sfromPort);
    onLog("Client  " + speer + " connected");
    if (!m_clients.contains(speer)) {
        emit newClient(pSocket->peerAddress());
        connect(pSocket, &QWebSocket::textMessageReceived, this, &WSServer::processTextMessage);
        connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WSServer::processBinaryMessage);
        connect(pSocket, &QWebSocket::disconnected, this, &WSServer::socketDisconnected);
        //TODO: when sendTextMessage following will also trigger!
        connect(pSocket, &QWebSocket::bytesWritten, this, &WSServer::onBytesWritten);

        m_clients[speer] =  pSocket;
        m_currentClient = speer; // TODO: this will be change when any client connected!!
    }else{
        onLog("m_clients already contain:" + speer);
    }

}
//! [onNewConnection]
//!
void WSServer::onClosed()
{
    disconnect(m_pWebSocketServer,&QWebSocketServer::newConnection, 0 ,0);
    disconnect(m_pWebSocketServer,&QWebSocketServer::closed, 0, 0);
    disconnect(m_pWebSocketServer,&QWebSocketServer::sslErrors, 0 ,0);
    disconnect(m_pWebSocketServer,&QWebSocketServer::serverError, 0 ,0);

}


//! [processTextMessage]
void WSServer::processTextMessage(QString message)
{
    emit actMessage(message);
}
//! [processTextMessage]

//! [processBinaryMessage]
void WSServer::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        pClient->sendBinaryMessage(message);
    }
}
//! [processBinaryMessage]

//! [socketDisconnected]
void WSServer::socketDisconnected()
{
    onLog("Client disconnected");
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        QString sfrom = pClient->peerAddress().toString();
        QString sfromPort = QString::number(pClient->peerPort());
        QString speer= QString("%1:%2").arg(sfrom, sfromPort);
        if (m_clients.contains(speer)) {
            qDebug() << "socketDisconnected: remove " << speer;
            m_clients.remove(speer);
        }else{
            qDebug() << "m_clients does not have " << speer;
        }
        pClient->deleteLater();
        pClient = nullptr;
    }
}
//! [socketDisconnected]

void WSServer::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "Ssl errors occurred" << errors;
}

void WSServer::onServerError(QWebSocketProtocol::CloseCode closeCode)
{
    qDebug() << "Server Error occurred:" << closeCode;
}

void WSServer::onBytesWritten(qint64 bytes)
{
    Q_UNUSED(bytes);
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        if (m_sendtype==WSServer::sendtype::file){
            QString target = pClient->peerAddress().toString();
//            qDebug() << "onBytesWritten: " << target << " size: " << QString::number(bytes);
            sendNextChunk(target);
        }
    }

}

void WSServer::sendNextChunk(QString target)
{
    //target: ip:port
    if (!m_clients.contains(target)){
        qDebug() << "No target client: " << target;
        return;
    }
    QWebSocket *client = m_clients.value(target);

    if (!client || m_files.isEmpty()) {
        return;
    }

    if (!m_currentFile) {
        m_currentFile = m_files.dequeue();
        m_fileName = QFileInfo(m_currentFile->fileName()).fileName();
        m_filenameSent = false;
    }

    if (m_currentFile->atEnd()) {
        int rc;
        qDebug() << "File transfer completed for" << m_fileName;
        rc = m_filenames.removeAll(m_fileName);
        if (rc > 1){
            qDebug() << "[sendNextChunk]remove: " << m_fileName << " more then one";
        }
        m_currentFile->close();
        delete m_currentFile;
        m_currentFile = nullptr;
        if (m_files.isEmpty()) {
            return;
        }
        m_currentFile = m_files.dequeue();
        m_fileName = QFileInfo(m_currentFile->fileName()).fileName();
        m_filenameSent = false;
    }

    QByteArray buffer;
    if (!m_filenameSent) {
        buffer = m_fileName.toUtf8() + '\0';
        m_filenameSent = true;
    }
    qint64 rc;
    buffer.append(m_currentFile->read(m_chunkSize - buffer.size()));
    rc = client->sendBinaryMessage(buffer);
//    qDebug() << "Sent chunk of size:" << buffer.size() << "for file:" << m_fileName;
    if (rc != buffer.size()){
        qDebug() << "send chunk of size error: \nexpect: " << QString::number(buffer.size()) <<
                    "\nactually: " << QString::number(rc);
    }

}

void WSServer::updateListen()
{
    if (m_pWebSocketServer->isListening()){
        m_pWebSocketServer->close();
    }
    // if (m_pWebSocketServer->listen(m_addr, m_port))
    if (m_pWebSocketServer->listen(QHostAddress::Any, m_port))
    {
        qInfo() << "WS Server listening on port" << m_port << " URL:" << m_pWebSocketServer->serverUrl();
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WSServer::onClosed);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors, this, &WSServer::onSslErrors);
        connect(m_pWebSocketServer, &QWebSocketServer::serverError, this, &WSServer::onServerError);
    }else {
        qDebug() << "Fail start WebSocketServer on port:" << QString::number(m_port);
    }
}

void WSServer::sendTextResult(QString msg)
{
    //send Test back to client
    qint64 rc = sendTextMessage(msg);
    if (rc<0){
        qDebug() << "sendTextResult: sendTextMessage return size:(" << rc << "):" << msg;
    }
}

bool WSServer::setIfname(QString mgr_ifname)
{
    m_ifname = mgr_ifname;
    QList<QHostAddress> addrs;
    addrs = m_myinfo->getIPfromIfname(m_ifname);
    emit onUpdateInterface(); // no need manager interface
    if (addrs.length()>0){
        m_addr = addrs[0]; // ip address
        // emit onUpdateInterface();
        return true;
    }else{
        qDebug() <<"setIfname: Did not get IP Address from interface:" << mgr_ifname;
        return false;
    }
}

