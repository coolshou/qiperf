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
