TEMPLATE = app
QT += core gui
QT += network

include(httpd.pri)

FORMS += \
    mainwindow.ui

HEADERS += \
    mainwindow.h

SOURCES += \
    main.cpp \
    mainwindow.cpp

