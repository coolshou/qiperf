
DEFINES += NOCRYPT
# USE_FILE32API  => file open limit to 2G

HEADERS += \
    src/asmZip.h
SOURCES += \
    src/asmZip.cpp

# unzip stuff
INCLUDEPATH += zlib/contrib/minizip

HEADERS += \
	zlib/contrib/minizip/ioapi.h \
	zlib/contrib/minizip/unzip.h \
	zlib/zconf.h \
	zlib/crc32.h \
	zlib/zutil.h \
	zlib/inftrees.h \
	zlib/inffast.h

SOURCES += \
	zlib/contrib/minizip/ioapi.c \
	zlib/contrib/minizip/unzip.c \
	zlib/inflate.c \
	zlib/adler32.c \
	zlib/crc32.c \
	zlib/zutil.c \
	zlib/inftrees.c \
	zlib/inffast.c
