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

QT_USE_NAMESPACE

//! [constructor]
WSClient::WSClient(QString serverip, const QUrl &url, QObject *parent) :
    QObject(parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WSClient::onDisconnected);
    //connect(&m_webSocket, &QWebSocket::errorOccurred, this, &WSClient::onErrorOccurred); // QT6.5
    connect(&m_webSocket, &QWebSocket::aboutToClose, this, &WSClient::onAboutToClose);
    connect(&m_webSocket, &QWebSocket::stateChanged, this, &WSClient::onStateChanged);
    connect(&m_webSocket, QOverload<const QList<QSslError>&>::of(&QWebSocket::sslErrors),
            this, &WSClient::onSslErrors);
//    qDebug() << "WSClient open websocket:" << url << Qt::endl;
    m_serverip = serverip;
    m_url = url;
    m_webSocket.open(m_url);
}
//! [constructor]

qint64 WSClient::sendText(QString message)
{
    qint64 rc=0;
    rc = m_webSocket.sendTextMessage(message);
    if (rc <=0){
        qDebug() << "error sendText size=" << rc << ", " << message;
    }
    return rc;
}

bool WSClient::isConnected()
{
    return m_webSocket.isValid();
}


//! [onConnected]
void WSClient::onConnected()
{
//    qDebug() << "WebSocket connected: " << m_url;
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &WSClient::onTextMessageReceived);
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
//    qDebug() << "WebSocket onStateChanged: "  << m_url << " : " << state;
    Q_UNUSED(state)
}


//! [onTextMessageReceived]
void WSClient::onTextMessageReceived(QString message)
{
    int cut2;
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
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
    }else if (act.startsWith(CMD_IPERF_STOPED)){
        cut2 = message.indexOf(':', 0);  //
        QString err_no = message.left(cut2); // error code
        message = message.right(message.length()-cut2-1); // error message
        qDebug()<< "CMD_IPERF_STOPED:" << message;
        emit iperfStoped(m_idx, err_no, message);

    } else if (act.startsWith(CMD_IPERF_TP_DATA)){
        QJsonParseError error;
        int cut3 = message.indexOf(':', 0);
        QString sInterval = message.left(cut3);
        message = message.right(message.length()-cut3-1);

        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError){
            emit iperfTPdata(m_idx, sInterval, message);
        }else{
            qDebug() << "onWSactMessage: ERROR: " + error.errorString() + "\nparser json: " + message.toUtf8();
        }
//        emit iperfStarted();
    } else {
        qDebug() << "Message received:" << message << ": "<< pClient->peerAddress();
    }
}
//! [onTextMessageReceived]

void WSClient::onSslErrors(const QList<QSslError> &errors)
{
//    Q_UNUSED(errors)
    qDebug() << "onSslErrors:" << errors;
    // WARNING: Never ignore SSL errors in production code.
    // The proper way to handle self-signed certificates is to add a custom root
    // to the CA store.
    m_webSocket.ignoreSslErrors();
}

