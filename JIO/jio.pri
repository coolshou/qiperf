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
    $$PWD/dlgbeamcmd.cpp \
    $$PWD/dlgcyntec.cpp \
    $$PWD/dlghanwha.cpp \
    $$PWD/dlgjio.cpp \
    $$PWD/dlgoptimize.cpp \
    $$PWD/dlgset.cpp \
    $$PWD/frmbeamtable.cpp \
    $$PWD/hanwha.cpp \
    $$PWD/lbrrestclient.cpp \
    $$PWD/optimizemodel.cpp \
    $$PWD/optimizeworker.cpp
    # $$PWD/../src/wsclient.cpp
    #$$PWD/../qiperfc/lib/qcpitemtriangle.cpp

HEADERS += \
    $$PWD/agmsensor.h \
    $$PWD/aip.h \
    $$PWD/beamdistance.h \
    $$PWD/beamtyperange.h \
    $$PWD/cyntec.h \
    $$PWD/cyntecbeamfactordata.h \
    $$PWD/cyntecbeamtabledata.h \
    $$PWD/dlgaip.h \
    $$PWD/dlgbeamcmd.h \
    $$PWD/dlgcyntec.h \
    $$PWD/dlghanwha.h \
    $$PWD/dlgjio.h \
    $$PWD/dlgoptimize.h \
    $$PWD/dlgset.h \
    $$PWD/frmbeamtable.h \
    $$PWD/hanwha.h \
    $$PWD/hanwhabeamtabledata.h \
    $$PWD/jiocmd.h \
    $$PWD/lbrrestclient.h \
    $$PWD/optimizedata.h \
    $$PWD/optimizeitem.h \
    $$PWD/optimizemodel.h \
    $$PWD/optimizeworker.h
    # $$PWD/../src/wsclient.h
    # $$PWD/../qiperfc/lib/qcpitemtriangle.h

FORMS += \
    $$PWD/dlgaip.ui \
    $$PWD/dlgbeamcmd.ui \
    $$PWD/dlgcyntec.ui \
    $$PWD/dlghanwha.ui \
    $$PWD/dlgjio.ui \
    $$PWD/dlgoptimize.ui \
    $$PWD/dlgset.ui \
    $$PWD/frmbeamtable.ui

RESOURCES += \
    $$PWD/aip.qrc \
    $$PWD/jio.qrc

DISTFILES += \
    $$PWD/cyntec.json \
    $$PWD/hanwha.json \
    $$PWD/jiocmd.json
