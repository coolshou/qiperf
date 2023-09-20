QT += core gui
QT += network
QT += printsupport # require by qcustomplot
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
include(../jcon-cpp.pri)

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
    lib/axistag.cpp \
    lib/qcustomplot.cpp \
    src/dlgiperf.cpp \
    src/endpointmgr.cpp \
    src/formendpoints.cpp \
    src/formoption.cpp \
    src/main.cpp \
    src/qiperfc.cpp \
    src/tp.cpp \
    src/tpmgr.cpp \
    src/udpreceiver.cpp

#    src/tpchart.cpp
#!android:{
#    SOURCES += \
#        src/formconsole.cpp \
#        src/tpchart.cpp
#}
HEADERS += \
    $$PWD/../src/pipeclient.h \
    $$PWD/../src/comm.h \
    $$PWD/../src/endpoint.h \
    $$PWD/../src/endpointtype.h \
    $$PWD/../src/versions.h \
    lib/axistag.h \
    lib/qcustomplot.h \
    src/dlgiperf.h \
    src/endpointmgr.h \
    src/formendpoints.h \
    src/formoption.h \
    src/qiperfc.h \
    src/tp.h \
    src/tpmgr.h \
    src/udpreceiver.h

#    src/tpchart.h
#!android:{
#    HEADERS += \
#        src/formconsole.h \
#        src/tpchart.h
#}
FORMS += \
    src/dlgiperf.ui \
    src/formendpoints.ui \
    src/formoption.ui \
    src/qiperfc.ui

#!android:{
#    src/formconsole.ui \
#}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../qiperf.qrc

#win32:VERSION = 1.2022.12.20 # major.minor.patch.build
#else:VERSION = 1.0.0    # major.minor.patch
win32 {
    #VER = $$system(findstr /c:"\"define QIPERFD_VERSION\"" $$PWD/../src/versions.h)
    VERSION = 0.2.11209.20
} else{
    VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFC_VERSION\"" | awk -F\' \'  \'{print $3}\' )
}

win32 {
# windows resources
#    CONFIG += embed_manifest_exe

    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qiperfc console of iperf" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/../images/qiperf.icon

    #DIST_DIRECTORY =  $$shell_quote($$shell_path($${ROOT_DIRECTORY}/../$${TARGET}_$${QT_ARCH}-$${VERSION}))
    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/../release/$${TARGET}.exe)) $$DIST_FILE
    deploy.commands = \
        windeployqt $$DIST_FILE

    first.depends = $(first) iperfbin deploy
    export(first.depends)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfbin deploy

}
unix:!android {
    DESKTOP.files += \
        qiperfc.desktop
    DESKTOP.path += \
        "/usr/share/applications/"

    IMAGES.files += \
        ../images/qiperf.png
    IMAGES.path += \
        "/usr/share/pixmaps/"
    INSTALLS += DESKTOP IMAGES
}
