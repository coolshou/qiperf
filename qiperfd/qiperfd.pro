# QT -= gui
QT += widgets
QT += core network websockets
QT += serialport


CONFIG += c++17 console
CONFIG -= app_bundle

#following setting will overwrite qtcreater's setting
#CONFIG += release
#CONFIG += debug

#include(../jcon-cpp.pri)
include(../qiperf.pri)
include(../lib/qntp/qntp.pri)
unix {
include(../QCtrlSignals/qctrlsignals.pri)
#include(../sigwatch.pri)

}
unix:!android {
    #LIBS += -lsystemd
    CONFIG += link_pkgconfig
    PKGCONFIG += libsystemd
    LIBS += $$system(pkg-config --libs libsystemd)
}
win32:{
    LIBS += -lws2_32
    LIBS += -liphlpapi
    LIBS += -lwbemuuid
    LIBS += -lsetupapi
    LIBS += -lole32 -loleaut32
    LIBS += -ladvapi32
    LIBS += -L$$(WindowsSdkDir)Include\$$(WindowsSDKLibVersion)\um -lwbemidl
    # -lcomsuppw //mingw not support
}

# QSSH
include(../lib/qssh/qssh.pri)  # this will cause compile error??why
# Don't clutter the example
DEFINES -= QT_NO_CAST_FROM_ASCII
DEFINES -= QT_NO_CAST_TO_ASCII
# require compile qssh.pro first (lib/qssh/lib/libQSsh.a)
# or cd lib/qssh; dpkg-buildpackage -b --no-sign
INCLUDEPATH += ../lib/qssh/src/libs/
win32:{
LIBS += -L$$OUT_PWD/../lib/qssh/lib/ \
     $$OUT_PWD/../lib/qssh/lib/QSsh.lib
}
# QSSH END
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../lib/ntp/ntpserver.cpp \
    ../lib/ntp/ntpsync.cpp \
    ../src/endpointtype.cpp \
    ../src/myfunc.cpp \
    ../src/icmpping.cpp \
    ../src/icmpwrapper.cpp \
    ../src/iperfwrapper.cpp \
    ../src/filewatcher.cpp \
    src/fileclient.cpp \
    src/iperfworker.cpp \
    src/main.cpp \
    src/myinfo.cpp \
    src/myservice.cpp \
    src/pipeserver.cpp \
    src/qiperfd.cpp \
    src/virtualdevice.cpp \
    src/virtualdevicetcp.cpp \
    src/serial/comdeviceserial.cpp \
    src/serial/serialtask.cpp \
    src/ssh/sshdeviceshell.cpp \
    src/ssh/sshtask.cpp \
    src/udpsrv.cpp \
    src/wsserver.cpp

#$$PWD/../src/sighandler.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/qiperf/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    ../lib/ntp/ntpserver.h \
    ../lib/ntp/ntpsync.h \
    ../src/endpointtype.h \
    ../src/myfunc.h \
    ../src/icmpping.h \
    ../src/comm.h \
    ../src/icmpwrapper.h \
    ../src/versions.h \
    ../src/iperfwrapper.h \
    ../src/filewatcher.h \
    ../src/tpmgrdata.h \
    src/fileclient.h \
    src/iperfworker.h \
    src/myinfo.h \
    src/myservice.h \
    src/pipeserver.h \
    src/qiperfd.h \
    src/virtualdevice.h \
    src/virtualdevicetcp.h \
    src/serial/comdeviceserial.h \
    src/serial/serialtask.h \
    src/ssh/sshdeviceshell.h \
    src/ssh/sshtask.h \
    src/udpsrv.h \
    src/version.h \
    src/wsserver.h
    #
    #$$PWD/../src/sighandler.h

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    DISTFILES += \
        android/AndroidManifest.xml \
        android/build.gradle \
        android/res/values/libs.xml \
        android/gradle.properties \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradlew \
        android/gradlew.bat
    RESOURCES += \
        android.qrc

}

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


VERSION = $$extract_version(14)
#VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFD_VERSION\"" | awk -F\' \'  \'{print $3}\' | awk -F\'\"\'  \'{print $2}\')
message(QIPERFD_VERSION: $$VERSION)

win32 {
    CONFIG += windeployqt
# windows resources
    RC_ICONS=$$PWD/../images/qiperfd.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qiperfd daemon of iperf client/server launcher" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    CONFIG(debug, debug|release) {
        DESTDIR = Debug
    } else {
        DESTDIR = Release
    }

    DISTFILES += $$PWD/../images/qiperfd.ico

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfdata.commands = \
        $$sprintf($$QMAKE_MKDIR_CMD, $$DIST_DIRECTORY) $$escape_expand(\\n\\t) \
        $$QMAKE_COPY_DIR $$shell_quote($$shell_path($$PWD/windows/)) $$shell_quote($$shell_path($$DIST_DIRECTORY/windows/))
CONFIG(release, debug|release) {
    release: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Release/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
} else {
    debug: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Debug/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
}

    first.depends = $(first) iperfdata iperfbin
    export(first.depends)
    export(iperfdata.commands)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfdata iperfbin

}
macx {
# Mac OS
    ICON = $$PWD/images/qiperf.icns
}

unix:!android {

    SERVICE_FILES.files +=\
        linux/qiperfd.service
    SERVICE_FILES.path += /lib/systemd/system/

    IMAGES_FILES.files = \
        ../images/qiperfd.png
    IMAGES_FILES.path += /usr/share/pixmaps/
    #
    INSTALLS += SERVICE_FILES IMAGES_FILES
    contains(QT_ARCH, aarch64||arm64) {
        B_ARCH="arm64"
    }
    contains(QT_ARCH, armv7||arm) {
        B_ARCH="armhf"
    }
    contains(QT_ARCH, x86_64) {
        B_ARCH="x86_64"
    }
    contains(QT_ARCH, i386) {
        B_ARCH="x86"
    }
    IPERF_FILES.files +=\
        linux/$$B_ARCH/iperf2 \
        linux/$$B_ARCH/iperf2.1 \
        linux/$$B_ARCH/iperf2.2.n \
        linux/$$B_ARCH/iperf3
    IPERF_FILES.path += /opt/qiperf/bin/linux/
    INSTALLS += IPERF_FILES

    if (contains($$B_ARCH,"")) {
        message("NOT support platform: " QT_ARCH)
    }
}

DISTFILES += \
    src/ws.cert \
    src/ws.key

RESOURCES += \
    src/qiperfd.qrc

