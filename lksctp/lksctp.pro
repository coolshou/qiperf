# 1. Project Type
# Defines this project as a shared library
TEMPLATE = lib

# 2. Target Name
# The name of the resulting library file (e.g., libmysctp.so)
TARGET = sctp
CONFIG += staticlib
CONFIG += openssl-linked

#include(../android_openssl/openssl.pri)
# include(lksctp.pri)

# 3. Compiler Flags
# Necessary flags for compiling C-specific code
QMAKE_CFLAGS_RELEASE += -std=gnu99
QMAKE_CFLAGS_DEBUG += -std=gnu99

# 4. Source Files
SCTP_SOURCE_DIR = sctp

# List all C source files from the lksctp source distribution.
# **NOTE:** You MUST update this list with the actual file names
# from the lksctp source you are using.
SOURCES += \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/opt_info.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/recvmsg.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/bindx.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/connectx.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/peeloff.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/sendmsg.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/lib/addrs.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/testlib/sctputil.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/withsctp/sctp_load_libs.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/withsctp/sctp_bind.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/withsctp/sctp_socket.c \
    $$PWD/$$SCTP_SOURCE_DIR/src/withsctp/sctp_sockopt.c


# 5. Header Files
# List all necessary header files
HEADERS += \
    $$PWD/config.h \
    $$PWD/$$SCTP_SOURCE_DIR/src/apps/sctp_darn.h \
    $$PWD/$$SCTP_SOURCE_DIR/src/withsctp/sctp_socket.h
    # $$PWD/$$SCTP_SOURCE_DIR/config.h \
    # $$PWD/$$SCTP_SOURCE_DIR/src/include/netinet/sctp.h \

# 6. Include Paths
# Where the compiler should look for the header files
INCLUDEPATH += \
    $$PWD/$$SCTP_SOURCE_DIR/ \
    $$PWD/$$SCTP_SOURCE_DIR/src/include/ \
    $$PWD/$$SCTP_SOURCE_DIR/src/testlib/

# 7. Configuration
# Optional: Add 'static' to build a static library (.a) instead of a shared library (.so)
# CONFIG += static

# 8. Deployment (Optional)
# Specify where the resulting library should be placed
# DESTDIR = lib

DESTDIR = $$PWD/../lib/$$QT_ARCH/

# # 1. Define the expected "ugly" name and the "clean" name
# # Adjust 'sctp_arm64-v8a' if your compiler uses a different suffix
# UGLY_NAME = libsctp_$${QT_ARCH}.a
# CLEAN_NAME = libsctp.a
# # 2. Create the move command based on the OS
# win32 {
#     # Windows uses 'move'
#     QMAKE_POST_LINK += move /Y \"$$DESTDIR/$$UGLY_NAME\" \"$$DESTDIR/$$CLEAN_NAME\"
# } else {
#     # Linux/macOS/Android Toolchain uses 'mv'
#     QMAKE_POST_LINK += mv -f \"$$DESTDIR/$$UGLY_NAME\" \"$$DESTDIR/$$CLEAN_NAME\"
# }
