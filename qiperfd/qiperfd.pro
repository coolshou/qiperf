QT -= gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        src/iperfworker.cpp \
        src/main.cpp \
        src/pipeserver.cpp \
        src/qiperfd.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    src/iperfworker.h \
    src/pipeserver.h \
    src/qiperfd.h \
    ../src/comm.h

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

win32:VERSION = 1.2022.12.20 # major.minor.patch.build
else:VERSION = 1.0.0    # major.minor.patch

win32 {
# windows resources
    CONFIG += embed_manifest_exe

    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_COMPANY="coolshou.idv.tw" #：指定項目目標的公司名稱，僅適用於Windows
    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qt base iperf client/server launcher" #：指定項目目標的描述資訊，僅適用於Windows
    QMAKE_TARGET_COPYRIGHT="Copyright 2022 coolshou.idv.tw" #：指定項目目標的版權資訊，僅適用於Windows
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

    RESOURCES += \
        linux.qrc
}

RESOURCES += \
    ../qiperf.qrc
