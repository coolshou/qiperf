#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Jun 26 09:31:22 2019

@author: jimmy
"""


import sys
import signal
import time
import unittest


try:
    from PyQt5.QtCore import (QCoreApplication, QUrl, QEventLoop, QThread,
    QDateTime)
    from PyQt5.QtWebSockets import (QWebSocket)
    #//, QWebSocketProtocol)
    from PyQt5.QtNetwork import QNetworkProxy, QAbstractSocket
    # python3-pyqt5.qtwebsockets

except ImportError as err:
    SystemExit("sudo apt install python3-pyqt5.qtwebsockets \n %s" % err)
# try:
#     from PyQt6.QtCore import (QCoreApplication, QUrl, QEventLoop)
#     from PyQt6.QtWebSockets import (QWebSocket, QWebSocketProtocol)
#     from PyQt6.QtNetwork import QNetworkProxy, QAbstractSocket
#     # python3-pyqt6.qtwebsockets
# except ImportError as err:
#     SystemExit("sudo apt install python3-pyqt6.qtwebsockets \n %s" % err)


class Application(QCoreApplication):
    """wrapper to the QApplication """

    def event(self, event_):
        """handle event """
        return QCoreApplication.event(self, event_)


def signal_handler(signal_, frame):
    """signal handler"""
    print('You pressed Ctrl+C!')
    sys.exit(0)


def on_finished(iCode, msg):
    print("[on_finished]%s: %s" % (iCode, msg))


def on_date(tid, ipall, data):
    '''iperf live data line'''
    print("[_on_date]%s:%s: %s" % (tid, ipall, data))


def on_result(row, col, tid, iType, msg):
    print("on_result: %s,%s : (%s) %s => %s" % (row, col, tid, iType, msg))


def on_debug(tpy, msg):
    print("on_debug:(%s) %s" % (tpy, msg))


def on_error(row, col, sType, sMsg):
    print("on_error:(%s, %s) %s: %s" % (row, col, sType, sMsg))

DATETIME_NOW_FORMAT="yyyy-MM-dd_hhmmss.zzz"

class WebSocketTest(unittest.TestCase):
    '''    test case for IperfClient class '''

    def setUp(self):
        self.APP = Application(sys.argv)
        # Connect your cleanup function to signal.SIGINT
        signal.signal(signal.SIGINT, signal_handler)
        # And start a timer to call Application.event repeatedly.
        # You can change the timer parameter as you like.
        self.APP.startTimer(200)
        # test code
        # self.wl = Wlan()
        # self.client = QWebSocket("",QWebSocketProtocol.Version13,None)
        self.init_client()
        self.init_client2()

        #connect
        #self.managerIP = "192.168.70.24"
        #self.bindIP = "192.168.111.125"
        self.port = 5201
        self.managerIP = "192.168.70.147"
        self.bindIP = self.managerIP #"192.168.0.198"
        self.bUseClient = False
        if self.bUseClient:
            self.client_managerIP = "192.168.70.21"
            self.client_bindIP = self.client_managerIP
        url = "ws://%s:47016/" % (self.managerIP)
        print("open %s" % url)
        self.client.open(QUrl(url))
        while self.client.state() != QAbstractSocket.SocketState.ConnectedState:
            print("wait client connect")
            time.sleep(1)
            QCoreApplication.processEvents(QEventLoop.ProcessEventsFlag.AllEvents)
        if self.bUseClient:
            url2 = "ws://%s:47016/" % (self.client_managerIP)
            print("open %s" % url2)
            self.client2.open(QUrl(url2))
            while self.client2.state() != QAbstractSocket.SocketState.ConnectedState:
                print("wait client2 connect")
                time.sleep(1)
                QCoreApplication.processEvents(QEventLoop.ProcessEventsFlag.AllEvents)
        self.currtime = QDateTime.currentDateTime().toString(DATETIME_NOW_FORMAT);

    def init_client(self):
        self.client = QWebSocket()
        proxy = QNetworkProxy(QNetworkProxy.ProxyType.NoProxy)
        self.client.setProxy(proxy)
        # self.client.errorOccurred.connect(self.on_error)
        self.client.connected.connect(self.on_connected)
        self.client.disconnected.connect(self.on_disconnected)
        self.client.textMessageReceived.connect(self.on_message_received)
        self.client.error.connect(self.on_error)
        self.client.stateChanged.connect(self.on_statechanged)

    def init_client2(self):
        self.client2 = QWebSocket()
        proxy = QNetworkProxy(QNetworkProxy.ProxyType.NoProxy)
        self.client2.setProxy(proxy)
        # self.client.errorOccurred.connect(self.on_error)
        self.client2.connected.connect(self.on_connected)
        self.client2.disconnected.connect(self.on_disconnected)
        self.client2.textMessageReceived.connect(self.on_message_received)
        self.client2.error.connect(self.on_error)
        self.client2.stateChanged.connect(self.on_statechanged)

    def test_websocket(self):
        '''Test websocket connect'''
        # self.client
        # self.client.pong.connect(self.onPong)
        print("send test")
        self.send_message("test")
        QThread.sleep(5)

    def test_ws_addIperf(self):
        # add iperf & start it
        cmd = "IPERF_ADD:0:{\"bidir\":false,\"bind\":\"%s\",\"delaytime\":0,\"fmtreport\":\"m\",\"interval\":1,\"manager\":\"%s\",\"parallel\":1,\"port\":%s,\"protocal\":\"TCP\",\"reverse\":false,\"server\":true,\"version\":\"3\"}" % (self.bindIP, self.managerIP, self.port)
        self.send_message(cmd)
        QThread.sleep(5)
        cmd = "IPERF_START:%s" % self.currtime
        self.send_message(cmd)
        QThread.sleep(5)

    def test_ws_addIperfClient(self):
        # add iperf client (require test_ws_addIperf)
        cmd = "IPERF_ADD:0:{\"bidir\":false,\"bind\":\"%s\",\"delaytime\":0,\"fmtreport\":\"m\",\"interval\":1,\"manager\":\"%s\",\"parallel\":1,\"port\":%s,\"protocal\":\"TCP\",\"reverse\":false,\"server\":false,\"version\":\"3\"}" % (self.client_bindIP, self.client_managerIP, self.port)
        self.send_message2(cmd)
        QThread.sleep(5)
        cmd = "IPERF_START:%s" % self.currtime
        self.send_message2(cmd)
        QThread.sleep(5)

    def test_ws_stopIperf(self):
        cmd = "IPERF_STOP:%s" % (self.managerIP)
        self.send_message(cmd)
        QThread.sleep(5)

    def test_ws_addSSH(self):
        # add ssh client
        cmd = "SSH_ADD:0:192.168.70.24:22:test:123456::30"
        self.send_message2(cmd)
        QThread.sleep(5)

    def send_message(self, msg):
        if self.client.isValid():
            print("client: send_message: %s" % msg)
            self.client.sendTextMessage(msg)
        else:
            print("WebSocket is not connected.")

    def send_message2(self, msg):
        if self.client2.isValid():
            print("client: send_message: %s" % msg)
            self.client2.sendTextMessage(msg)
        else:
            print("WebSocket is not connected.")

    def on_connected(self):
        print("WebSocket connected")

    def on_disconnected(self):
        print("WebSocket disconnected")

    def on_error(self, error):
        print("on_error: %s" % error)

    def on_statechanged(self, status):
        print("on_statechanged: (%s) %s" % (status, self.get_state(status)))

    def get_state(self, status):
        msg = ""
        if status == QAbstractSocket.SocketState.UnconnectedState:
            msg = "UnconnectedState"
        if status == QAbstractSocket.SocketState.HostLookupState:
            msg = "HostLookupState"
        if status == QAbstractSocket.SocketState.ConnectingState:
            msg =  "ConnectingState"
        if status == QAbstractSocket.SocketState.ConnectedState:
            msg = "ConnectedState"
        if status == QAbstractSocket.SocketState.BoundState:
            msg = "BoundState"
        if status == QAbstractSocket.SocketState.ClosingState:
            msg = "ClosingState"
        if status == QAbstractSocket.SocketState.ListeningState:
            msg = "ListeningState"

        return msg

    def on_message_received(self, message):
        print("on_message_received: %s" % message)

    def error(self, error_code):
        print("error code: {}".format(error_code))
        print(self.client.errorString())

    def onPong(self, elapsedTime, payload):
        print("onPong - time: {} ; payload: {}".format(elapsedTime, payload))

    def close(self):
        self.client.close()
        self.client2.close()

    def tearDown(self):
        '''
        clean up when test finish
        '''
        self.close()
        del self.APP


if __name__ == '__main__':
    suite = unittest.TestSuite()
    # suite.addTest(WebSocketTest('test_websocket'))
    #suite.addTest(WebSocketTest('test_ws_addIperf'))
    #suite.addTest(WebSocketTest('test_ws_addIperfClient'))
    #suite.addTest(WebSocketTest('test_ws_stopIperf'))
    suite.addTest(WebSocketTest('test_ws_addSSH'))

    unittest.TextTestRunner(verbosity=2).run(suite)
