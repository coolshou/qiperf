# https://asmaloney.com/2011/12/code/in-memory-zip-file-access-using-qt/
QT += core

TARGET = asmZip
TEMPLATE = app
TARGETDIR=example
include(asmZip.pri)

INCLUDEPATH += \
    src
OBJECTS_DIR = obj

SOURCES += \
    example/main.cpp

mac {
   LIBS	+= -lz
}
