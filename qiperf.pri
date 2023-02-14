INCLUDEPATH += \
    $$PWD/src


isEmpty(DESTDIR) {
    CONFIG(debug, debug|release) {
        DESTDIR=$$PWD/Debug
    }
    CONFIG(release, debug|release) {
        DESTDIR=$$PWD/Release
    }
}


win32 {
    # windows resources
    CONFIG += embed_manifest_exe
    QMAKE_TARGET_COMPANY="alphanetworks.com" #：指定項目目標的公司名稱，僅適用於Windows
    QMAKE_TARGET_COPYRIGHT="Copyright 2023 alphanetworks.com" #：指定項目目標的版權資訊，僅適用於Windows

}
