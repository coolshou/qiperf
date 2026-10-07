TARGET = iperf3
TEMPLATE = lib

CONFIG+=staticlib

include(iperf3.pri)

DESTDIR = $$PWD/../lib
