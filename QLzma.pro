#-------------------------------------------------
#
# Project created by QtCreator 2011-04-23T22:48:26
#
#-------------------------------------------------

QT       += core gui

TARGET = QLzma
TEMPLATE = app
DEFINES += _7ZIP_ST

INCLUDEPATH += lzma/C
SOURCES += main.cpp\
    lzma/C/LzmaLib.c \
    lzma/C/LzmaEnc.c \
    lzma/C/LzmaDec.c \
    lzma/C/Lzma2Enc.c \
    lzma/C/Lzma2Dec.c \
    lzma/C/LzFind.c \
    lzma/C/Alloc.c \
    qlzma.cpp

HEADERS  += \
    lzma/C/LzmaLib.h \
    lzma/C/LzmaEnc.h \
    lzma/C/LzmaDec.h \
    lzma/C/Lzma2Enc.h \
    lzma/C/Lzma2Dec.h \
    lzma/C/LzFind.h \
    lzma/C/Alloc.h \
    qlzma.h \
    lzma/C/Types.h \
    global.h

FORMS    +=

CONFIG += mobility
MOBILITY = 

symbian {
    TARGET.UID3 = 0xe55709e1
    # TARGET.CAPABILITY += 
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000
}

OTHER_FILES += \
    lzma/lzma.txt

