TEMPLATE = lib
QT -= gui
CONFIG += lzma-buildlib
DEFINES += _7ZIP_ST

include(lzma.pri)

HEADERS = C/LzmaLib.h \
    C/LzmaEnc.h \
    C/LzmaDec.h \
    C/Lzma2Enc.h \
    C/Lzma2Dec.h \
    C/LzFind.h \
    C/Alloc.h \
    C/Types.h


SOURCES = C/LzmaLib.c \
    C/LzmaEnc.c \
    C/LzmaDec.c \
    C/Lzma2Enc.c \
    C/Lzma2Dec.c \
    C/LzFind.c \
    C/Alloc.c

