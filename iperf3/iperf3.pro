TARGET = iperf3
TEMPLATE = lib

CONFIG+=staticlib

include(iperf3.pri)
include($$PWD/../lksctp/lksctp.pri)
DESTDIR = $$PWD/../lib
