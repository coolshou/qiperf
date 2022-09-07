QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/iperfworker.cpp \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/iperfworker.h \
    src/mainwindow.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

    DISTFILES += \
        android/AndroidManifest.xml \
        android/build.gradle \
        android/res/values/libs.xml

    RESOURCES += \
            android.qrc

}

win32:VERSION = 1.0.2022.09 # major.minor.patch.build
else:VERSION = 1.0.0    # major.minor.patch

win32 {
# windows resources
    CONFIG += embed_manifest_exe

    RC_ICONS=$$PWD/images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_COMPANY="coolshou.idv.tw" #：指定項目目標的公司名稱，僅適用於Windows
    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qt base iperf server launcher" #：指定項目目標的描述資訊，僅適用於Windows
    QMAKE_TARGET_COPYRIGHT="Copyright © 2022 coolshou.idv.tw" #：指定項目目標的版權資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/images/qiperf.icon
}
macx {
# Mac OS
    ICON = $$PWD/images/qiperf.icns
}

RESOURCES += \
    linux.qrc \
    qiperf.qrc

