
include($$PWD/httpdcore.pri)

QT       += gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
    $$PWD

SOURCES += \
    $$PWD/myhttpserverform.cpp

HEADERS += \
    $$PWD/myhttpserverform.h

FORMS += \
    $$PWD/myhttpserverform.ui

RESOURCES += \
    $$PWD/myhttpserver.qrc
