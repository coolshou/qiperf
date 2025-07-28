QT += core gui
QT += network websockets
QT += printsupport # require by qcustomplot
QT += webenginewidgets
QT += serialport
QT += positioning # for QGeoCoordinate


#following setting will overwrite qtcreater's setting
#CONFIG += release
#CONFIG += debug

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
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
# greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11
lessThan(QT_MAJOR_VERSION, 5): QMAKE_CXXFLAGS += -std=c++11
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat # requite by QT6 QTextCodec

win32:unix:!android:{
    QT += charts
}

# following will cause QCustomPlot double free on APP exit!!
CONFIG += c++17

include(../qiperf.pri)
include(../lib/qgeoview.pri)
unix {
#include(../sigwatch.pri)
}

#GeographicLib
# cmake .. -DBUILD_SHARED_LIBS=OFF
INCLUDEPATH += \
    $$PWD/../lib/geographiclib/build/include \
    $$PWD/../lib/geographiclib/include
unix {
LIBS += \
    -L$$PWD/../lib/geographiclib/build/src -lGeographicLib
}
# cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="/EHsc /wd4819  /wd4456  /wd4244 /WX-"  ..
# msbuild -p:Configuration=Release GeographicLib.sln
# msbuild -p:Configuration=Debug GeographicLib.sln
win32 {
    CONFIG(debug, debug|release) {
        LIBS += \
            -L$$PWD/../lib/geographiclib/build/lib/Debug -lGeographicLib
    }else{
        LIBS += \
            -L$$PWD/../lib/geographiclib/build/lib/Release -lGeographicLib
    }
}

INCLUDEPATH += \
    $$PWD/lib \
    $$PWD/lib/QXlsx/QXlsx/header

GEOFILES.file += \
    $$PWD/../lib/geoids/egm96-5.pgm \
    $$PWD/../lib/geoids/egm96-5.pgm.aux.xml \
    $$PWD/../lib/geoids/egm96-5.wld
GEOFILES.path += \
    "/opt/qiperf/bin/geoids"

# for excel
QXLSX_PARENTPATH=$$PWD/lib/QXlsx/QXlsx         # current QXlsx path is . (. means curret directory)
QXLSX_HEADERPATH=$$PWD/lib/QXlsx/QXlsx/header/  # current QXlsx header path is ./header/
QXLSX_SOURCEPATH=$$PWD/lib/QXlsx/QXlsx/source/  # current QXlsx source path is ./source/
include($$PWD/lib/QXlsx/QXlsx/QXlsx.pri)

# debug
#CONFIG += sanitizer
#CONFIG += sanitize_address

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../JIO/agmsensor.cpp \
    ../src/cpumonitor.cpp \
    ../src/dlgshowlog.cpp \
    ../src/filewatcher.cpp \
    ../src/gps/geotranslate.cpp \
    ../src/gps/gpsfunc.cpp \
    ../src/map/dlggeoosm.cpp \
    ../src/myfunc.cpp \
    ../src/pipeclient.cpp \
    ../src/endpoint.cpp \
    ../src/endpointtype.cpp \
    ../src/endpointact.cpp \
    ../src/iperfwrapper.cpp \
    ../src/icmpping.cpp \
    ../src/icmpwrapper.cpp \
    ../src/port/portsetbox.cpp \
    ../src/port/serialport.cpp \
    ../src/port/tcpudpport.cpp \
    auto/dlgsimplemicro.cpp \
    auto/simpleworker.cpp \
    lib/axistag.cpp \
    lib/geoview/placemark.cpp \
    lib/geoview/polyline.cpp \
    lib/geoview/rectangletext.cpp \
    lib/myqcpbars.cpp \
    lib/myqcpgraph.cpp \
    lib/qcustomplot.cpp \
    src/codeeditor.cpp \
    src/customheaderview.cpp \
    src/dlgintbox.cpp \
    src/dlgiperf.cpp \
    src/dlgiperfrestartrule.cpp \
    src/dlgoption.cpp \
    src/dlgping.cpp \
    src/dlgrecord.cpp \
    src/dlgserial.cpp \
    src/dlgssh.cpp \
    src/dlgtest.cpp \
    src/endpointmgr.cpp \
    src/exporthtml.cpp \
    src/filesaveSocket.cpp \
    src/fileserver.cpp \
    src/formqiperfds.cpp \
    src/iperffileworker.cpp \
    src/main.cpp \
    src/nmessagebox.cpp \
    src/nowrapdelegate.cpp \
    src/numberdelegate.cpp \
    src/pingitem.cpp \
    src/pingmgr.cpp \
    src/pingplot.cpp \
    src/qipconfig.cpp \
    src/qiperfc.cpp \
    src/tp.cpp \
    src/tpflowmanager.cpp \
    src/tpfoldingdelegate.cpp \
    src/tpmgr.cpp \
    src/tpdirdelegate.cpp \
    src/tpplot.cpp \
    src/tpworker.cpp \
    src/udpreceiver.cpp \
    src/wsclient.cpp \
    src/tooltipeventfilter.cpp \
    views/sshview.cpp \
    views/terminal/highlighter.cpp \
    views/terminal/qvterminal/qvtchar.cpp \
    views/terminal/qvterminal/qvtcharformat.cpp \
    views/terminal/qvterminal/qvterminal.cpp \
    views/terminal/qvterminal/qvtlayout.cpp \
    views/terminal/qvterminal/qvtline.cpp \
    views/terminal/terminalview.cpp \
    views/terminal/termview.cpp \
    views/viewmanager.cpp \
    views/serialview.cpp \
    views/throughputview.cpp
#    src/tpstatuschecker.cpp
#    ../src/map/dlgopenstreetmap.cpp
SOURCES += \
    ../JIO/aip.cpp \
    ../JIO/dlgaip.cpp \
    ../JIO/dlgjio.cpp \
    ../JIO/cyntec.cpp \
    ../JIO/hanwha.cpp \
    ../JIO/lbrrestclient.cpp

    # lib/tplegenditem.cpp \
    # ../qiperfd/src/wsserver.cpp \

HEADERS += \
    ../src/cpumonitor.h \
    ../src/dlgshowlog.h \
    ../src/filewatcher.h \
    ../src/gps/geotranslate.h \
    ../src/gps/gpsfunc.h \
    ../src/map/dlggeoosm.h \
    ../src/pipeclient.h \
    ../src/comm.h \
    ../src/endpoint.h \
    ../src/endpointtype.h \
    ../src/port/abstractport.h \
    ../src/port/portsetbox.h \
    ../src/port/serialport.h \
    ../src/port/tcpudpport.h \
    ../src/versions.h \
    ../src/endpointact.h \
    ../src/iperfwrapper.h \
    ../src/icmpping.h \
    ../src/icmpwrapper.h \
    ../src/myfunc.h \
    ../src/tpmgrdata.h \
    auto/dlgsimplemicro.h \
    auto/simpleworker.h \
    lib/axistag.h \
    lib/geoview/placemark.h \
    lib/geoview/polyline.h \
    lib/geoview/rectangletext.h \
    lib/myqcpbars.h \
    lib/myqcpgraph.h \
    lib/qcustomplot.h \
    src/codeeditor.h \
    src/customheaderview.h \
    src/dlgintbox.h \
    src/dlgiperf.h \
    src/dlgiperfrestartrule.h \
    src/dlgoption.h \
    src/dlgping.h \
    src/dlgrecord.h \
    src/dlgserial.h \
    src/dlgssh.h \
    src/dlgtest.h \
    src/endpointmgr.h \
    src/exporthtml.h \
    src/filesaveSocket.h \
    src/fileserver.h \
    src/formqiperfds.h \
    src/iperffileworker.h \
    src/nmessagebox.h \
    src/nowrapdelegate.h \
    src/numberdelegate.h \
    src/pingitem.h \
    src/pingmgr.h \
    src/pingplot.h \
    src/qipconfig.h \
    src/qiperfc.h \
    src/tp.h \
    src/tpflowmanager.h \
    src/tpfoldingdelegate.h \
    src/tpgroup.h \
    src/tpmgr.h \
    src/tpdirdelegate.h \
    src/tpplot.h \
    src/tpstatus.h \
    src/tpworker.h \
    src/udpreceiver.h \
    src/wsclient.h \
    src/tooltipeventfilter.h \
    views/abstractview.h \
    views/serialdata.h \
    views/sshdata.h \
    views/sshview.h \
    views/terminal/highlighter.h \
    views/terminal/qvterminal/qvtchar.h \
    views/terminal/qvterminal/qvtcharformat.h \
    views/terminal/qvterminal/qvterminal.h \
    views/terminal/qvterminal/qvtlayout.h \
    views/terminal/qvterminal/qvtline.h \
    views/terminal/terminalview.h \
    views/terminal/termview.h \
    views/viewmanager.h \
    views/serialview.h \
    views/throughputview.h \
    views/viewtype.h \
    plugin/plugininterface.h

#    src/tpstatuschecker.h

HEADERS += \
    ../JIO/aip.h \
    ../JIO/dlgaip.h \
    ../JIO/dlgjio.h \
    ../JIO/cyntec.h \
    ../JIO/hanwha.h \
    ../JIO/lbrrestclient.h \
    ../JIO/agmsensor.h \
    ../JIO/cyntecbeamfactordata.h \
    ../JIO/cyntecbeamtabledate.h

#    ../src/map/dlgopenstreetmap.h
# lib/tplegenditem.h \
    # ../qiperfd/src/wsserver.h \

FORMS += \
    ../src/dlgshowlog.ui \
    ../src/map/dlggeoosm.ui \
    ../src/port/portsetbox.ui \
    ../src/port/serialport.ui \
    ../src/port/tcpudpport.ui \
    auto/dlgsimplemicro.ui \
    src/dlgintbox.ui \
    src/dlgiperf.ui \
    src/dlgiperfrestartrule.ui \
    src/dlgoption.ui \
    src/dlgping.ui \
    src/dlgrecord.ui \
    src/dlgserial.ui \
    src/dlgssh.ui \
    src/dlgtest.ui \
    src/formqiperfds.ui \
    src/qiperfc.ui \
    views/abstractview.ui \
    views/serialview.ui \
    views/sshview.ui \
    views/throughputview.ui

#    ../src/map/dlgopenstreetmap.ui
FORMS += \
    ../JIO/dlgaip.ui \
    ../JIO/dlgjio.ui

UI_DIR= \
    $$PWD/ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}
else: unix:!android: target.path = /opt/qiperf/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../qiperf.qrc \
    ../JIO/aip.qrc \
    lib/geoview/placemark.qrc

# Define a function to extract the version
defineReplace(extract_version) {
    line_number = $$1
    # Read the contents of the file into a variable
    contents = $$cat($$PWD/../src/versions.h)
    # Split the file contents into lines
    lines = $$split(contents, "\n")
    # Get the specific line (line numbers are zero-based, so we subtract 1)
    result = $$member(lines, $$line_number)
    # Extract the version
    version = $$replace(result, \", )
    # Return the extracted version
    return($$version)
}
# get git branch & version
GITBRANCH = $$system(git rev-parse --abbrev-ref HEAD)
DEFINES += GITBRANCH=\\\"$$GITBRANCH\\\"
GITVER = $$system(git rev-parse --short=8 HEAD)
DEFINES += GITVER=\\\"$$GITVER\\\"

# Set the VERSION variable
VERSION = $$extract_version(20)
#VERSION = $$system(cat $$PWD/../src/versions.h | grep "\"define QIPERFC_VERSION\"" | awk -F\' \'  \'{print $3}\' | awk -F\'\"\'  \'{print $2}\')
message(QIPERFC_VERSION: $$VERSION)

# template.files += \
#         template/result.html


win32 {
    # windows resources
    #    CONFIG += embed_manifest_exe
    CONFIG += windeployqt

    RC_ICONS=$$PWD/../images/qiperf.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    #QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'

    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="quick iperf console" #：指定項目目標的描述資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    CONFIG(debug, debug|release) {
        DESTDIR = Debug
    } else {
        DESTDIR = Release
    }

    DISTFILES += $$PWD/../images/qiperf.ico

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${PWD}/../$${TARGET}_$${QT_ARCH}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    iperfdata.commands = \
        $$sprintf($$QMAKE_MKDIR_CMD, $$DIST_DIRECTORY) $$escape_expand(\\n\\t)
CONFIG(release, debug|release) {
    release: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Release/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
} else {
    debug: iperfbin.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/Debug/$${TARGET}.exe)) $$shell_quote($$shell_path($$DIST_FILE))
}
    #template.path += $${DIST_DIRECTORY}/template/
    template.commands = \
        $$QMAKE_COPY $$shell_quote($$shell_path($${PWD}/template/result.html)) $$shell_quote($$shell_path($${DIST_DIRECTORY}/template/))
    # INSTALLS += template

    first.depends = $(first) iperfbin #template
    export(first.depends)
    export(iperfdata.commands)
    export(iperfbin.commands)
    # export(template.commands)
    QMAKE_EXTRA_TARGETS += first iperfbin #template

}
unix:!android {
    MIME.files += \
        alphanetworks-qiperfc.xml
    MIME.path += \
        "/usr/share/mime/packages/"

    ICONS.files += \
        ../images/qiperfc.png \
        ../images/qiperfcs.png
    ICONS.path += \
        "/usr/share/icons/"

    DESKTOP.files += \
        qiperfc.desktop
    DESKTOP.path += \
        "/usr/share/applications/"

    IMAGES.files += \
        ../images/qiperfc.png \
        ../images/qiperfcs.png
    IMAGES.path += \
        "/usr/share/pixmaps/"

    #template.path += /opt/qiperf/template/

    INSTALLS += MIME ICONS DESKTOP IMAGES #template
}

INSTALLS += GEOFILES
