QT       += gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += \
    $$PWD/httpd

SOURCES += \
    $$PWD/httpd/myhttpserverform.cpp

HEADERS += \
    $$PWD/httpd/myhttpserver.h \
    $$PWD/httpd/myhttpserverform.h

FORMS += \
    $$PWD/httpd/myhttpserverform.ui

RESOURCES += \
    $$PWD/httpd/myhttpserver.qrc
