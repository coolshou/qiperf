
include($$PWD/httpdcore.pri)

QT       += gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
    $$PWD/src

SOURCES += \
    $$PWD/src/myhttpserverform.cpp

HEADERS += \
    $$PWD/src/myhttpserverform.h

FORMS += \
    $$PWD/src/myhttpserverform.ui

RESOURCES += \
    $$PWD/myhttpserver.qrc
