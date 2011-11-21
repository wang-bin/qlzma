#include <cstdio>
#include <qapplication.h>
#include <qprogressdialog.h>
#include "qlzma.h"



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QLzma lzma;
	lzma.setUncompressedFile(argv[1]);
	lzma.compress();

	qDebug("(lzma 4.32.7) unpack size: %d, pack size: %d", lzma.unpackSize(), lzma.packSize());

	return a.exec();
}
