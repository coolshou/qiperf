

#qgeoview
include(qgeoview/samples/lib.pri)
INCLUDEPATH += \
    $$PWD/qgeoview/lib/include/ \
    $$PWD/qgeoview/lib/include/QGeoView/

DEFINES += QGV_EXPORT

HEADERS += \
    $$PWD/qgeoview/lib/include/QGeoView/QGVCamera.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVDrawItem.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVGlobal.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVUtils.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVItem.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayer.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerBing.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerGoogle.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerOSM.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerBDGEx.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerTiles.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVLayerTilesOnline.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVMap.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVMapQGItem.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVMapQGView.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVMapRubberBand.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVProjection.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVProjectionEPSG3857.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVWidget.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVWidgetCompass.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVWidgetScale.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVWidgetText.h \
    $$PWD/qgeoview/lib/include/QGeoView/QGVWidgetZoom.h \
    $$PWD/qgeoview/lib/include/QGeoView/Raster/QGVImage.h \
    $$PWD/qgeoview/lib/include/QGeoView/Raster/QGVIcon.h

SOURCES += \
    $$PWD/qgeoview/lib/src/QGVCamera.cpp \
    $$PWD/qgeoview/lib/src/QGVDrawItem.cpp \
    $$PWD/qgeoview/lib/src/QGVGlobal.cpp \
    $$PWD/qgeoview/lib/src/QGVUtils.cpp \
    $$PWD/qgeoview/lib/src/QGVItem.cpp \
    $$PWD/qgeoview/lib/src/QGVLayer.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerBing.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerGoogle.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerOSM.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerBDGEx.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerTiles.cpp \
    $$PWD/qgeoview/lib/src/QGVLayerTilesOnline.cpp \
    $$PWD/qgeoview/lib/src/QGVMap.cpp \
    $$PWD/qgeoview/lib/src/QGVMapQGItem.cpp \
    $$PWD/qgeoview/lib/src/QGVMapQGView.cpp \
    $$PWD/qgeoview/lib/src/QGVMapRubberBand.cpp \
    $$PWD/qgeoview/lib/src/QGVProjection.cpp \
    $$PWD/qgeoview/lib/src/QGVProjectionEPSG3857.cpp \
    $$PWD/qgeoview/lib/src/QGVWidget.cpp \
    $$PWD/qgeoview/lib/src/QGVWidgetCompass.cpp \
    $$PWD/qgeoview/lib/src/QGVWidgetScale.cpp \
    $$PWD/qgeoview/lib/src/QGVWidgetText.cpp \
    $$PWD/qgeoview/lib/src/QGVWidgetZoom.cpp \
    $$PWD/qgeoview/lib/src/Raster/QGVImage.cpp \
    $$PWD/qgeoview/lib/src/Raster/QGVIcon.cpp

# shared
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

