
QT += opengl
DEFINES += QCUSTOMPLOT_USE_OPENGL # qcustomplot use OPENGL
unix:!android {
    # 22.04 (freeglut3-dev)
    INCLUDEPATH +=/usr/include/GL/
    LIBS += -lglut
    # -lOpenGL
    # 24.04 (libglut-dev)
    #CONFIG += link_pkgconfig
    #PKGCONFIG += glut
}
win32: {
    LIBS += \
        -lOpengl32 \
        -lglu32
    LIBS += -lws2_32
}

HEADERS += \
	$$PWD/qcpitemtriangle.h \
	$$PWD/qcustomplot.h \
	$$PWD/axistag.h \
	$$PWD/myqcpbars.h \
	$$PWD/myqcpgraph.h

SOURCES += \
	$$PWD/qcpitemtriangle.cpp \
	$$PWD/qcustomplot.cpp \
	$$PWD/axistag.cpp \
	$$PWD/myqcpbars.cpp \
	$$PWD/myqcpgraph.cpp
