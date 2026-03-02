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
        android/res/values/libs.xml \
        android/gradle/wrapper/gradle-wrapper.properties \
        android/gradle/wrapper/gradle-wrapper.jar \
        android/res/drawable-hdpi/icon.png \
        android/res/drawable-ldpi/icon.png \
        android/res/drawable-mdpi/icon.png \
        android/res/drawable-xhdpi/icon.png \
        android/res/drawable-xxhdpi/icon.png \
        android/res/drawable-xxxhdpi/icon.png \
        raw/arm/iperf2 \
        raw/arm/iperf3 \
        raw/arm64/iperf2 \
        raw/arm64/iperf3 \
        raw/x86/iperf2 \
        raw/x86/iperf3 \
        raw/x86_64/iperf2 \
        raw/x86_64/iperf3

}

RESOURCES += \
    qiperf.qrc

DISTFILES += \
    android/settings.gradle


