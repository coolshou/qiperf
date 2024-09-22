TEMPLATE = subdirs

SUBDIRS += \
    qiperfd \
    qiperftray

SUBDIRS += \
    qiperfc \

qiperftray.subdir = qiperftray
qiperfd.subdir = qiperfd
qiperfc.subdir = qiperfc

#qiperfc.depends = qiperfd
qiperftray.depends = qiperfd


DEBIAN.files += \
    debian/source/format \
    debian/changelog \
    debian/control \
    debian/copyright \
    debian/rules \
    debian/README.Debian \
    debian/README.source \
    debian/qiperfc.control \
    debian/qiperfc.install \
    debian/qiperfc.links \
    debian/qiperfd.control \
    debian/qiperfd.install \
    debian/qiperfd.postinst \
    debian/qiperfd.preinst \
    debian/qiperftray.control \
    debian/qiperftray.install \
    debian/qiperftray.links \
    TODO \
    test.sh


NSIS.files += \
    qiperf.nsi
