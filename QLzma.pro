TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = lzma qlzma

qlzma.file = src/qlzma.pro
qlzma.depends += lzma


OTHER_FILES += \
    lzma/lzma.txt
