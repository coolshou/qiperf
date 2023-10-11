QT -= gui
QT += core network websockets

CONFIG += c++17 console
CONFIG -= app_bundle

#unix {
    include(../jcon-cpp.pri)
#} else {
#    INCLUDEPATH += \
#        ../jcon-cpp/src/
#}

include(../qiperf.pri)
#include(../sigwatch.pri)

unix:!android {
    #LIBS += -lsystemd
    CONFIG += link_pkgconfig
    PKGCONFIG += libsystemd
    LIBS += $$system(pkg-config --libs libsystemd)
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    $$PWD/../src/endpointtype.cpp \
    src/iperfworker.cpp \
    src/main.cpp \
    src/myinfo.cpp \
    src/myservice.cpp \
    src/pipeserver.cpp \
    src/qiperfd.cpp \
    src/udpsrv.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    $$PWD/../src/endpointtype.h \
    $$PWD/../src/comm.h \
    $$PWD/../src/versions.h \
    src/iperfworker.h \
    src/myinfo.h \
    src/myservice.h \
    src/pipeserver.h \
    src/qiperfd.h \
    src/udpsrv.h \
    src/version.h

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

#win32:VERSION = 1.2023.2.14 # major.minor.patch.build
#else:VERSION = 1.0.0    # major.minor.patch

win32 {
    #VER = $$system(findstr /c:"\"define QIPERFD_VERSION\"" $$PWD/../src/versions.h)
    VERSION = 0.2.11209.20
} else{
    VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFD_VERSION\"" | awk -F\' \'  \'{print $3}\' )
}

win32 {
# windows resources
    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qiperfd daemon of iperf client/server launcher" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/../images/qiperf.icon

    #DIST_DIRECTORY =  $$shell_quote($$shell_path($${ROOT_DIRECTORY}/../$${TARGET}_$${QT_ARCH}-$${VERSION}))
    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfdata.commands = \
        $$sprintf($$QMAKE_MKDIR_CMD, $$DIST_DIRECTORY) $$escape_expand(\\n\\t) \
        $$QMAKE_COPY_DIR $$shell_quote($$shell_path($$PWD/windows/)) $$shell_quote($$shell_path($$DIST_DIRECTORY/windows/))
    iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/../release/$${TARGET}.exe)) $$DIST_FILE
    deploy.commands = \
        windeployqt $$DIST_FILE

    first.depends = $(first) iperfdata iperfbin deploy
    export(first.depends)
    export(iperfdata.commands)
    export(iperfbin.commands)
    QMAKE_EXTRA_TARGETS += first iperfdata iperfbin deploy

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
    contains(QT_ARCH, aarch64) {
        B_ARCH="arm64"
        RESOURCES += \
            linux-arm64.qrc
    }
    contains(QT_ARCH, x86_64) {
        B_ARCH="x86_64"
        RESOURCES += \
            linux.qrc
    }
    contains(QT_ARCH, i386) {
        B_ARCH="i686"
        RESOURCES += \
            linux-i686.qrc
    }

    if (contains($$B_ARCH,"")) {
        message("NOT support platform: " QT_ARCH)

    }
}

