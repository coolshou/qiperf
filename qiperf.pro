TEMPLATE = subdirs

SUBDIRS += \
    qiperfd \
    qiperftray

SUBDIRS += \
    qiperfc \

qiperftray.subdir = qiperftray
qiperfd.subdir = qiperfd
qiperfc.subdir = qiperfc

qiperfc.depends = qiperfd
qiperftray.depends = qiperfd


unix:!android {
    DEB_FILES.files += \
        debian/changelog \
        debian/control \
        debian/copyright \
        debian/qiperfc.install \
        debian/qiperfc.link \
        debian/qiperfd.install \
        debian/qiperfd.link \
        debian/qiperfd.preinst \
        debian/qiperfd.postinst \
        debian/qiperftray.install \
        debian/qiperftray.link \
        debian/rules \
        debian/README.Debian \
        debian/README.source \
        debian/source/format
}
