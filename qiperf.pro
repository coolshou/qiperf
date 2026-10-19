QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# include(lksctp/lksctp.pri)
include(iperf3/iperf3.pri)

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/mainwindow.h

FORMS += \
    src/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    # android 15 (API 35) above require 16KB memory page size
    QMAKE_LFLAGS += -Wl,-z,max-page-size=16384 -Wl,-z,common-page-size=16384

    DISTFILES += \
        android/AndroidManifest.xml \
        android/build.gradle \
        android/proguard-rules.pro \
        android/res/values/libs.xml \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/res/drawable-hdpi/icon.png \
        android/res/drawable-ldpi/icon.png \
        android/res/drawable-mdpi/icon.png \
        android/res/drawable-xhdpi/icon.png \
        android/res/drawable-xxhdpi/icon.png \
        android/res/drawable-xxxhdpi/icon.png

    ANDROID_MIN_SDK_VERSION = 26
    ANDROID_TARGET_SDK_VERSION = 36  # Match your installed Android platform SDK version
}

RESOURCES += \
    qiperf.qrc

DISTFILES += \
    android/settings.gradle


# 1. 定義編譯另一個 .pro 的命令
lksctp.target = compile_lksctp
android {
    lksctp.commands = \
        cd $$PWD/lksctp && \
        /opt/Qt/6.10.3/android_arm64_v8a/bin/qmake -spec android-clang lksctp.pro && \
        $(MAKE) clean && \
        $(MAKE)
}else {
    lksctp.commands = cd $$PWD/lksctp && $(QMAKE) lksctp.pro && $(MAKE)
}

iperf3.target = compile_iperf3
android {
    iperf3.commands = \
        cd $$PWD/iperf3 && \
        /opt/Qt/6.10.3/android_arm64_v8a/bin/qmake -spec android-clang iperf3.pro && \
        $(MAKE) clean && \
        $(MAKE)
} else {
    iperf3.commands = cd $$PWD/iperf3 && $(QMAKE) iperf3.pro && $(MAKE)
}
# 2. 將自訂命令加入 Extra Targets
QMAKE_EXTRA_TARGETS += lksctp iperf3

# 3. 設定為本專案編譯前的前提依賴 (Pre-target dependency)
PRE_TARGETDEPS += compile_lksctp compile_iperf3

