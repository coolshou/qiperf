QT       += core
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = aas
TEMPLATE = lib

# Define as a plugin
CONFIG   += plugin

include(../lib/qssh/qssh.pri)
include(../qiperfc/lib/QXlsx/QXlsx/QXlsx.pri)
include(../lib/qgeoview.pri)
include(../lib/geographiclib.pri)
include(jio.pri)

INCLUDEPATH += \
    ../qiperfc/plugin/ \
    ../qiperfc/src/map/

HEADERS += \
    jioplugin.h \
    ../qiperfc/plugin/plugininterface.h \
    ../src/map/dlggeoosm.h \
    ../qiperfc/lib/geoview/rectangletext.h

SOURCES += \
	jioplugin.cpp \
	../src/map/dlggeoosm.cpp \
	../qiperfc/lib/geoview/rectangletext.cpp

FORMS += \
	../src/map/dlggeoosm.ui

# Add a JSON metadata file (optional but recommended)
# This file can contain additional information about your plugin
QMAKE_JSON_EXTENSIONS = \
	aasplugin.json

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}
else: unix:!android: target.path = /opt/qiperf/bin
!isEmpty(target.path): INSTALLS += target

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../Debug
} else {
    DESTDIR = $$PWD/../Release
}
