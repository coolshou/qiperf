TEMPLATE = subdirs

SUBDIRS += \
    qiperfd \
    qiperftray \
    qgeoview
#    qssh

SUBDIRS += \
    qiperfc \

qiperftray.subdir = qiperftray
qiperfd.subdir = qiperfd
qiperfc.subdir = qiperfc
qgeoview.subdir = lib/qgeoview/lib
#qssh.subdir = lib/qssh
qiperftray.depends = qiperfd
qiperfc.depends = qgeoview
#qiperfd.depends = qssh
# qssh.subdir = lib/qssh/src/libs/qssh
# qiperfd.depends = qssh

DEBIAN.files += \
    debian/changelog \

QIPERFCDEBIAN.files += \
    qiperfc/debian/control \
    qiperfc/debian/copyright \
    qiperfc/debian/rules \
    qiperfc/debian/README.Debian \
    qiperfc/debian/README.source \
    qiperfc/debian/qiperfc.links

QIPERFDDEBIAN.files += \
    qiperfd/debian/README.Debian \
    qiperfd/debian/control \
    qiperfd/debian/copyright \
    qiperfd/debian/rules \
    qiperfd/debian/README.source \
    qiperfd/debian/qiperfd.postinst \
    qiperfd/debian/qiperfd.preinst

QIPERFTRAYDEBIAN.files += \
    qiperftray/debian/control \
    qiperftray/debian/copyright \
    qiperftray/debian/rules \
    qiperftray/debian/README.Debian \
    qiperftray/debian/README.source \
    qiperftray/debian/qiperftray.links

MISC.files += \
    README.md \
    deploy.sh \
    build_x64.bat \
    build_setup.bat \
    build_deb.sh \
    TODO

NSIS.files += \
    qiperfd.nsi \
    qiperf.nsi

TEST.files += \
    pytest/test_qtwebsocket.py \
    pytest/test_websocket.py
