TEMPLATE = app
QT += core gui
QT += network

include(httpd.pri)

FORMS += \
    /srcmainwindow.ui

HEADERS += \
    /srcmainwindow.h

SOURCES += \
    /srcmain.cpp \
    /srcmainwindow.cpp

RESOURCES += \
    myhttpd.qrc

win32 {
    CONFIG += windeployqt
    RC_ICONS=$$PWD/images/http.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="simple http server" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    CONFIG(debug, debug|release) {
        DESTDIR = Debug
    } else {
        DESTDIR = Release
    }
    DISTFILES += $$PWD/images/http.ico

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))
    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    CONFIG(release, debug|release) {
        release: myhttpbin.commands = \
            $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Release/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
    } else {
        debug: myhttpbin.commands = \
            $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Debug/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
    }
    first.depends = $(first) myhttpbin
    export(first.depends)
    export(myhttpbin.commands)
    QMAKE_EXTRA_TARGETS += first myhttpbin
}
