TEMPLATE = subdirs

INCLUDEPATH += \
    src

SUBDIRS += qiperfd
SUBDIRS += qiperfc

#include(qiperfd/qiperfd.pro)
#include(qiperfc/qiperfc.pro)
