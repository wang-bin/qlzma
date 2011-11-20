#include <cstdio>
#include <qapplication.h>
#include <qprogressdialog.h>
#include "qlzma.h"

#include "lzma/C/Types.h"

#include "global.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QLzma lzma(argv[1]);
	lzma.compress();

	lzma.setInPath("lzma-4.32.7.tar.lzma");
	ZDEBUG("lzma-4.32.7.tar.lzma unpack size: %d",lzma.unpackSize());

	return 0;//a.exec();
}
