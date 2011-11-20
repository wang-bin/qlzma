TARGET = QLzma
TEMPLATE = app

include(../lzma/lzma.pri)

INCLUDEPATH += ..

SOURCES += main.cpp\
    qlzma.cpp

HEADERS  += \
    qlzma.h \
    global.h
