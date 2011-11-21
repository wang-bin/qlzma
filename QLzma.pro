TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = lzma qlzma

qlzma.file = src/qlzma.pro
qlzma.depends += lzma


OTHER_FILES += \
	README \
	TODO.txt \
	lzma/lzma.txt
