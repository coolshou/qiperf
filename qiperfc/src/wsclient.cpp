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
#include "wsclient.h"
#include <QtCore/QDebug>
#include <QtWebSockets/QWebSocket>
#include "comm.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QDir>
QT_USE_NAMESPACE

//! [constructor]
WSClient::WSClient(QString serverip, const QUrl &url, QString datapath, QObject *parent) :
    QObject(parent),m_serverip(serverip),m_url(url),m_datapath(datapath),
    m_webSocket(nullptr)
{
    m_webSocket = new QWebSocket();
    m_webSocket->setProxy(QNetworkProxy::NoProxy); // avoid to use proxy
    // QNetworkProxy qnp = m_webSocket->proxy();
    // qDebug() << "proxy hostName: " << qnp.hostName() << " capabilities(): " << qnp.capabilities();

    connect(m_webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &WSClient::onDisconnected);
    //connect(m_webSocket, &QWebSocket::errorOccurred, this, &WSClient::onErrorOccurred); // QT6.5
    connect(m_webSocket, &QWebSocket::aboutToClose, this, &WSClient::onAboutToClose);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &WSClient::onStateChanged);
    connect(m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &WSClient::onSslErrors);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WSClient::onError);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &WSClient::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, &WSClient::onBinaryMessageReceived);
    // m_serverip = serverip;
    // m_url = url;
//    m_datapath = datapath + QDir::separator();
    setDatapath(datapath);
    // qDebug() << "m_webSocket->open(m_url): " << m_url;
    m_webSocket->open(m_url);
}
//! [constructor]

qint64 WSClient::sendText(QString message)
{
    qint64 rc=0;
    if (m_webSocket->isValid()){
        rc = m_webSocket->sendTextMessage(message);
        if (rc <=0){
            qDebug() << "error sendText size=" << rc << ", " << message;
        }
    }
    return rc;
}

bool WSClient::isConnected()
{
    if (m_webSocket!=nullptr){
        if (m_webSocket->state() == QAbstractSocket::ConnectedState){
            return true;
        }else{
            return false;
        }
        // TODO: is isValid() ok for check the websocket connected!!??
        // return m_webSocket->isValid(); // qwebsocket is ready to read/write
    }else {
        qDebug() << "ERROR: m_webSocket not exist";
        return false;
    }
}

void WSClient::setDatapath(QString datapath)
{
    if (!datapath.endsWith(QDir::separator())) {
            datapath.append(QDir::separator());
    }
    m_datapath = datapath;
}


//! [onConnected]
void WSClient::onConnected()
{
    // qInfo() << "WebSocket connected: " << m_url;
}
//! [onConnected]
//!
void WSClient::onDisconnected()
{
//    qDebug() << "WebSocket Disconnected: " << m_url;
    emit disconnected(m_serverip);
}

void WSClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    qDebug() << "WebSocket onErrorOccurred: " << m_url << " : " << socketError;
}

void WSClient::onAboutToClose()
{
//    qDebug() << "WebSocket onAboutToClose";

}

void WSClient::onStateChanged(QAbstractSocket::SocketState state)
{
    // qDebug() << "WebSocket onStateChanged: "  << m_url << " : " << state;
   Q_UNUSED(state)
}


//! [onTextMessageReceived]
void WSClient::onTextMessageReceived(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString from = pClient->peerAddress().toString();
    // pClient->localAddress().toString();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        // qDebug() << "(TODO)from: " << from << " :json: " << message;
    }else{
        int cut2;

        int cut = message.indexOf(':', 0);
        QString act = message.left(cut); //action : CMD_IPERF_STARTED/CMD_IPERF_STOPED...
        message = message.right(message.length()-cut-1);
        cut2 = message.indexOf(':', 0);
        QString m_idx = message.left(cut2); // refrow
        message = message.right(message.length()-cut2-1);

        if (act.startsWith(CMD_IPERF_STARTED)){
            cut2 = message.indexOf(':', 0);
            QString smode = message.left(cut2); // S: server/ C: client mode
            message = message.right(message.length()-cut2-1); // key
            emit iperfStarted(smode, message);
        } else if (act.startsWith(CMD_IPERF_STOPED)){
            cut2 = message.indexOf(':', 0);  //
            QString err_no = message.left(cut2); // error code
            message = message.right(message.length()-cut2-1);
            cut2 = message.indexOf(':', 0);  //
            QString error = message.left(cut2); // error status
            QString bindkey = message.right(message.length()-cut2-1); // bindkey

            //        qDebug()<< "CMD_IPERF_STOPED:" << err_no << " bindkey:" << bindkey << " error:" << error;
            emit iperfStoped(m_idx, err_no, error, bindkey);
        } else if (act.startsWith(CMD_IPERF_ERRORED)){
            cut2 = message.indexOf(':', 0);  //
            QString err_no = message.left(cut2); // error code
            message = message.right(message.length()-cut2-1);
            cut2 = message.lastIndexOf(':');  //
            QString error = message.left(cut2); // key
            QString bindkey = message.right(message.length()-cut2-1); // error message
            //        qDebug()<< "CMD_IPERF_ERRORED:" << err_no << " bindkey:" << bindkey << " message:" << error;
            emit iperfStoped(m_idx, err_no, error, bindkey);

        } else if (act.startsWith(CMD_IPERF_TP_DATA)){
            // QJsonParseError error;
            int cut3 = message.indexOf(':', 0);
            QString sInterval = message.left(cut3);
            message = message.right(message.length()-cut3-1);

            doc = QJsonDocument::fromJson(message.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError){
                emit iperfTPdata(m_idx, sInterval, message);
            }else{
                qDebug() << "onWSactMessage: ERROR: " + error.errorString() + "\nparser json: " + message.toUtf8();
            }
            //        emit iperfStarted();
        } else {
            qDebug() << "Message received: act:" << act <<" refrow:" << m_idx <<
                " :"<< message << ": "<< from;
        }
    }
}
//! [onTextMessageReceived]

void WSClient::onBinaryMessageReceived(const QByteArray &message) {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    QString from = pClient->peerAddress().toString();

    if (!m_files.contains(from)){
        // Extract filename
        int nullIndex = message.indexOf('\0');
        if (nullIndex == -1) {
            qWarning() << "Invalid message format";
            return;
        }
        QString fileName = QString::fromUtf8(message.left(nullIndex));
        QString filePath = m_datapath + fileName;
        m_files[from] =new QFile(filePath);
        if (!m_files[from]->open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open file" << filePath << ":" << m_files[from]->errorString();
            delete m_files[from];
            m_files[from] = nullptr;
            return;
        }
        m_files[from]->write(message.mid(nullIndex + 1));
    } else {
        m_files[from]->write(message);
    }

//    qInfo() << "Chunk written to file";

    if (message.size() < m_chunkSize && m_files[from]) {
        // Assume that a smaller chunk means end of file
        qInfo() << "File transfer completed and saved: " << m_files[from]->fileName();
        m_files[from]->close();
        m_files.remove(from);
        delete m_files[from];
        m_files[from] = nullptr;
    }

}

void WSClient::onSslErrors(const QList<QSslError> &errors)
{
//    Q_UNUSED(errors)
    qDebug() << "onSslErrors:" << errors;
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    m_webSocket->ignoreSslErrors();
}

void WSClient::onError(QAbstractSocket::SocketError error)
{
    if (error != QAbstractSocket::RemoteHostClosedError){
        qDebug() << "[" <<m_webSocket->peerAddress().toString() << "]WSClient::onError:" << error ;
    }else{
        // TODO : handle RemoteHostClosedError

    }
    //TODO: handle websocket not connect issue
}

