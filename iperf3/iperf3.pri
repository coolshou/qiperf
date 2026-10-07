QMAKE_CFLAGS += -std=c17
QMAKE_CFLAGS += -std=gnu17
QMAKE_CXXFLAGS += -std=c++17
QT += core concurrent network

IPERF3_SOURCE_DIR = iperf3

INCLUDEPATH += \
    $$PWD \
    $$PWD/$$IPERF3_SOURCE_DIR/ \
    $$PWD/$$IPERF3_SOURCE_DIR/src

linux: {
    LIBS += -lm  -ldl
    android: {
        #https://github.com/KDAB/android_openssl
        CONFIG += openssl-linked
        include(../android_openssl/openssl.pri) # include openssl's so file
        if (versionAtLeast(QT_VERSION, 6.5.0)) {
            INCLUDEPATH += "$$PWD/../android_openssl/no-asm/ssl_3/include"
            LIBS += -L"$$PWD/../android_openssl/no-asm/ssl_3/$$QT_ARCH"
            message("LIBS:" $$LIBS)
        }else{
            INCLUDEPATH += "$$PWD/../android_openssl/no-asm/ssl_1.1/include"
            LIBS += -L"$$PWD/../android_openssl/no-asm/ssl_1.1/$$QT_ARCH"
        }
        LIBS += -lssl -lcrypto
    }else{
        # android not support sctp
        # with this, Linux will use sctp as default?
        # include(../lksctp/lksctp.pri) # require change iperf_config.h's setting
        LIBS += -lpthread
        # openssl
        PKGCONFIG += openssl
        LIBS += -lssl -lcrypto
    }
}

HEADERS += \
    $$PWD/$$IPERF3_SOURCE_DIR/iperf_config.h

SOURCES += \
    $$PWD/$$IPERF3_SOURCE_DIR/src/cjson.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/dscp.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_api.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_auth.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_client_api.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_error.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_locale.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_pthread.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_sctp.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_server_api.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_tcp.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_time.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_udp.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/iperf_util.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/net.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/tcp_info.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/timer.c \
    $$PWD/$$IPERF3_SOURCE_DIR/src/units.c


win32:{
    LIBS += -lws2_32 -lmswsock -lwsock32
}


