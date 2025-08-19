# cmake .. -DBUILD_SHARED_LIBS=OFF
INCLUDEPATH += \
    $$PWD/geographiclib/build/include \
    $$PWD/geographiclib/include
unix {
LIBS += \
    -L$$PWD/geographiclib/build/src -lGeographicLib
}
# cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="/EHsc /wd4819  /wd4456  /wd4244 /WX-"  ..
# msbuild -p:Configuration=Release GeographicLib.sln
# msbuild -p:Configuration=Debug GeographicLib.sln
win32 {
    CONFIG(debug, debug|release) {
        LIBS += \
            -L$$PWD/geographiclib/build/lib/Debug -lGeographicLib
    }else{
        LIBS += \
            -L$$PWD/geographiclib/build/lib/Release -lGeographicLib
    }
}
