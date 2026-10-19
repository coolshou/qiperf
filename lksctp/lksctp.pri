
SCTP_SOURCE_DIR = sctp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/$$SCTP_SOURCE_DIR/
INCLUDEPATH += $$PWD/$$SCTP_SOURCE_DIR/src/
INCLUDEPATH += $$PWD/$$SCTP_SOURCE_DIR/src/include/

SCTP_PATH = $$PWD/../lib

# Link the specific static library
android {
LIBS += $$SCTP_PATH/$$QT_ARCH/libsctp_arm64-v8a.a
} else {
LIBS += $$SCTP_PATH/$$QT_ARCH/libsctp.a
}

HEADERS += \
    $$PWD/config.h

SCTP_IN_FILE = $$PWD/sctp/src/include/netinet/sctp.h.in
SCTP_OUT_FILE = $$PWD/sctp/src/include/netinet/sctp.h

unix {
    sctp_header.commands = cp $$SCTP_IN_FILE $$SCTP_OUT_FILE
}
win32 {
    sctp_header.commands = copy /Y $$replace(SCTP_IN_FILE, /, \\) $$replace(SCTP_OUT_FILE, /, \\)
}

sctp_header.target = $$SCTP_OUT_FILE
sctp_header.depends = $$SCTP_IN_FILE

QMAKE_EXTRA_TARGETS += sctp_header
PRE_TARGETDEPS += $$SCTP_OUT_FILE