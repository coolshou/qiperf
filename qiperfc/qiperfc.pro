QT += core gui
QT += network websockets
QT += printsupport # require by qcustomplot
#CONFIG += release
CONFIG += debug

DEFINES += QCUSTOMPLOT_USE_OPENGL # qcustomplot use OPENGL
win32: {
    LIBS += \
        -lOpengl32 \
        -lglu32
}
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11


win32:unix:!android:{
    QT += charts
}

# following will cause QCustomPlot double free on APP exit!!
#CONFIG += c++17

include(../qiperf.pri)
#include(../jcon-cpp.pri)
unix {
#include(../sigwatch.pri)
}
INCLUDEPATH += \
    $$PWD/lib

# debug
#CONFIG += sanitizer
#CONFIG += sanitize_address

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/../src/pipeclient.cpp \
    $$PWD/../src/endpoint.cpp \
    $$PWD/../src/endpointtype.cpp \
    $$PWD/../src/endpointact.cpp \
    $$PWD/../src/iperfwrapper.cpp \
    lib/axistag.cpp \
    lib/qcustomplot.cpp \
    src/QIPConfig.cpp \
    src/codeeditor.cpp \
    src/customheaderview.cpp \
    src/dlgiperf.cpp \
    src/dlgoption.cpp \
    src/dlgrecord.cpp \
    src/dlgtest.cpp \
    src/endpointmgr.cpp \
    src/filesaveSocket.cpp \
    src/fileserver.cpp \
    src/formqiperfds.cpp \
    src/htmlwriter.cpp \
    src/iperffileworker.cpp \
    src/main.cpp \
    src/qiperfc.cpp \
    src/rpctp.cpp \
    src/tp.cpp \
    src/tpmgr.cpp \
    src/tpdirdelegate.cpp \
    src/tpplot.cpp \
    src/udpreceiver.cpp \
    src/wsclient.cpp \
    src/tooltipeventfilter.cpp

    # ../qiperfd/src/wsserver.cpp \

HEADERS += \
    $$PWD/../src/pipeclient.h \
    $$PWD/../src/comm.h \
    $$PWD/../src/endpoint.h \
    $$PWD/../src/endpointtype.h \
    $$PWD/../src/versions.h \
    $$PWD/../src/endpointact.h \
    $$PWD/../src/iperfwrapper.h \
    lib/axistag.h \
    lib/qcustomplot.h \
    src/QIPConfig.h \
    src/codeeditor.h \
    src/customheaderview.h \
    src/dlgiperf.h \
    src/dlgoption.h \
    src/dlgrecord.h \
    src/dlgtest.h \
    src/endpointmgr.h \
    src/filesaveSocket.h \
    src/fileserver.h \
    src/formqiperfds.h \
    src/htmlwriter.h \
    src/iperffileworker.h \
    src/qiperfc.h \
    src/rpctp.h \
    src/tp.h \
    src/tpmgr.h \
    src/tpdirdelegate.h \
    src/tpplot.h \
    src/udpreceiver.h \
    src/wsclient.h \
    src/tooltipeventfilter.h

    # ../qiperfd/src/wsserver.h \

FORMS += \
    src/dlgiperf.ui \
    src/dlgoption.ui \
    src/dlgrecord.ui \
    src/dlgtest.ui \
    src/formqiperfds.ui \
    src/qiperfc.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../qiperf.qrc

VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFC_VERSION\"" | awk -F\' \'  \'{print $3}\' | awk -F\'\"\'  \'{print $2}\')
message(QIPERFC_VERSION: $$VERSION)

win32 {
    #VER = $$system(findstr /c:"\"define QIPERFD_VERSION\"" $$PWD/../src/versions.h)
    #VERSION = 0.2.11306.27 # major.minor.patch.build
    # windows resources
    #    CONFIG += embed_manifest_exe

    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="quick iperf console" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/../images/qiperf.icon

    #DIST_DIRECTORY =  $$shell_quote($$shell_path($${ROOT_DIRECTORY}/../$${TARGET}_$${QT_ARCH}-$${VERSION}))
    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
CONFIG(release, debug|release) {
    release: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/../Release/$${TARGET}.exe)) $$DIST_FILE
} else {
    debug: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/../Debug/$${TARGET}.exe)) $$DIST_FILE
}
    deploy.commands = \
        windeployqt $$DIST_FILE

    first.depends = $(first) iperfbin deploy
    export(first.depends)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfbin deploy

}
unix:!android {
    MIME.files += \
        qiperfc.xml
    MIME.path += \
        "/usr/share/mime/packages/"

    ICONS.files += \
        ../images/qiperf.png
    ICONS.path += \
        "/usr/share/icons/"

    DESKTOP.files += \
        qiperfc.desktop
    DESKTOP.path += \
        "/usr/share/applications/"

    IMAGES.files += \
        ../images/qiperf.png
    IMAGES.path += \
        "/usr/share/pixmaps/"
    INSTALLS += MIME ICONS DESKTOP IMAGES
}
