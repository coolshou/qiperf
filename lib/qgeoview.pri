
include(qgeoview/samples/lib.pri)

INCLUDEPATH += \
    $$PWD/qgeoview/samples/shared/

SOURCES += \
    $$PWD/qgeoview/samples/shared/helpers.cpp \
    $$PWD/qgeoview/samples/shared/placemarkcircle.cpp \
    $$PWD/qgeoview/samples/shared/rectangle.cpp

HEADERS += \
    $$PWD/qgeoview/samples/shared/helpers.h \
    $$PWD/qgeoview/samples/shared/placemarkcircle.h \
    $$PWD/qgeoview/samples/shared/rectangle.h

