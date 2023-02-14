QT += core gui
QT += network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include(../qiperf.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mytray.cpp \
    $$PWD/../src/pipeclient.cpp \
    src/qiperftray.cpp

HEADERS += \
    src/mytray.h \
    $$PWD/../src/comm.h \
    $$PWD/../src/pipeclient.h \
    src/qiperftray.h

FORMS += \
    src/qiperftray.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../qiperf.qrc

win32:VERSION = 1.2023.2.14 # major.minor.patch.build
else:VERSION = 1.0.0    # major.minor.patch

win32 {
# windows resources

    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qiperftray qiperfd systemtray launcher" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/../images/qiperf.icon

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${ROOT_DIRECTORY}/../$${TARGET}_$${QT_ARCH}-$${VERSION}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfdata.commands = \
        $$sprintf($$QMAKE_MKDIR_CMD, $$DIST_DIRECTORY) $$escape_expand(\\n\\t) \
        $$QMAKE_COPY_DIR $$shell_quote($$shell_path($$PWD/windows/)) $$shell_quote($$shell_path($$DIST_DIRECTORY/windows/))
    iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}.exe)) $$DIST_FILE
    deploy.commands = \
        windeployqt $$DIST_FILE

    first.depends = $(first) iperfdata iperfbin deploy
    export(first.depends)
    export(iperfdata.commands)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfdata iperfbin deploy

}
unix:!android {
    DESKTOP.files += \
        qiperftray.desktop
    DESKTOP.path += \
        "/usr/share/applications/"

    IMAGES.files += \
        ../images/qiperf.png
    IMAGES.path += \
        "/usr/share/pixmaps/"
    INSTALLS += DESKTOP IMAGES
}

#DISTFILES += \
#    qiperftray.desktop
