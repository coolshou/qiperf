#!/usr/bin/env python3
# -*- coding: utf-8 -*-
try:
    #websocket-client
    from websocket import create_connection
except ImportError as err:
    SystemError(err)

def main(testadd, testdel=False):
    ws = create_connection("ws://192.168.70.147:47016")

    # print(ws.recv())
    if testadd:
        print("Sending 'SERIAL_ADD'...")
        ws.send("SERIAL_ADD:0:/dev/ttyUSB0:115200:8:0:1:0")

    if testdel:
        print("Sending 'SERIAL_DEL'...")
        ws.send("SERIAL_DEL:/dev/ttyUSB0")

    if testadd or testdel:
        print("Receiving...")
        result =  ws.recv()
        print("Received '%s'" % result)

    ws.close()

# main
if __name__ == '__main__':
    # main(True)
    main(False, True)

