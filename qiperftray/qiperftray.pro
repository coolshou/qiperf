QT += core gui
QT += network

#following setting will overwrite qtcreater's setting
#CONFIG += release
#CONFIG += debug

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

include(../qiperf.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../src/pipeclient.cpp \
    ../src/dlgshowlog.cpp \
    ../src/filewatcher.cpp \
    src/main.cpp \
    src/mytray.cpp \
    src/qiperftray.cpp

HEADERS += \
    ../src/comm.h \
    ../src/pipeclient.h \
    ../src/versions.h \
    ../src/dlgshowlog.h \
    ../src/filewatcher.h \
    src/mytray.h \
    src/qiperftray.h

FORMS += \
    ../src/dlgshowlog.ui \
    src/qiperftray.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/qiperf/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../qiperf.qrc

# Define a function to extract the version
defineReplace(extract_version) {
    line_number = $$1
    # Read the contents of the file into a variable
    contents = $$cat($$PWD/../src/versions.h)
    # Split the file contents into lines
    lines = $$split(contents, "\n")
    # Get the specific line (line numbers are zero-based, so we subtract 1)
    result = $$member(lines, $$line_number)
    # Extract the version
    version = $$replace(result, \", )
    # Return the extracted version
    return($$version)
}

# get git branch & version
GITBRANCH = $$system(git rev-parse --abbrev-ref HEAD)
DEFINES += GITBRANCH=\\\"$$GITBRANCH\\\"
GITVER = $$system(git rev-parse --short=8 HEAD)
DEFINES += GITVER=\\\"$$GITVER\\\"

VERSION = $$extract_version(26)
#VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFTRAY_VERSION\"" | awk -F\' \'  \'{print $3}\' | awk -F\'\"\'  \'{print $2}\')
message(QIPERFTRAY_VERSION: $$VERSION)

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

    CONFIG(debug, debug|release) {
        DESTDIR = Debug
    } else {
        DESTDIR = Release
    }

    DISTFILES += $$PWD/../images/qiperf.icon

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfdata.commands = \
        $$sprintf($$QMAKE_MKDIR_CMD, $$DIST_DIRECTORY) $$escape_expand(\\n\\t)
CONFIG(release, debug|release) {
    release: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Release/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
} else {
    debug: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Debug/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
}
    deploy.commands = \
        windeployqt $$shell_quote($$shell_path($$DIST_FILE))

    first.depends = $(first) iperfbin deploy
    export(first.depends)
    export(iperfdata.commands)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfbin deploy

}
unix:!android {
    DESKTOP.files += \
        qiperftray.desktop
    DESKTOP.path += \
        "/usr/share/applications/"

    IMAGES.files += \
        ../images/qiperftray.png
    IMAGES.path += \
        "/usr/share/pixmaps/"
    INSTALLS += DESKTOP IMAGES
}
