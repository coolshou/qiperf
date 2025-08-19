QT += core
QT += network
QT += positioning # for QGeoCoordinate
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat # requite by QT6 QTextCodec


INCLUDEPATH += \
    ../qiperfc \
    ../src

SOURCES += \
    $$PWD/agmsensor.cpp \
    $$PWD/aip.cpp \
    $$PWD/cyntec.cpp \
    $$PWD/dlgaip.cpp \
    $$PWD/dlgjio.cpp \
    $$PWD/dlgset.cpp \
    $$PWD/frmbeamtable.cpp \
    $$PWD/hanwha.cpp \
    $$PWD/lbrrestclient.cpp


HEADERS += \
    $$PWD/agmsensor.h \
    $$PWD/aip.h \
    $$PWD/cyntec.h \
    $$PWD/cyntecbeamfactordata.h \
    $$PWD/cyntecbeamtabledata.h \
    $$PWD/dlgaip.h \
    $$PWD/dlgjio.h \
    $$PWD/dlgset.h \
    $$PWD/frmbeamtable.h \
    $$PWD/hanwha.h \
    $$PWD/hanwhabeamtabledata.h \
    $$PWD/jiocmd.h \
    $$PWD/lbrrestclient.h

FORMS += \
    $$PWD/dlgaip.ui \
    $$PWD/dlgjio.ui \
    $$PWD/dlgset.ui \
    $$PWD/frmbeamtable.ui

RESOURCES += \
    $$PWD/aip.qrc \
