
SCTP_SOURCE_DIR = sctp

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/$$SCTP_SOURCE_DIR/src
INCLUDEPATH += $$PWD/$$SCTP_SOURCE_DIR/src/include/

SCTP_PATH = $$PWD/../lib

# Link the specific static library
LIBS += $$SCTP_PATH/$$QT_ARCH/libsctp.a
