TARGET = qlzma
TEMPLATE = app

include(../lzma/lzma.pri)

INCLUDEPATH += ..

SOURCES += main.cpp\
    qlzma.cpp \
    gui/ezprogressdialog.cpp \
    utils/convert.cpp \
    utils/qt_util.cpp

HEADERS  += \
    qlzma.h \
    qtcompat.h \
    gui/ezprogressdialog_p.h \
    gui/ezprogressdialog.h \
    utils/convert.h \
    utils/qt_util.h \
    msgdef.h







