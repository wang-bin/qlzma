#ifndef QLZMA_H
#define QLZMA_H

#include <vector>
#include <qstring.h>
#include "lzma/C/Types.h"
//struct ICompressProgress;
class QLzmaPrivate;
class QLzma
{
public:
	QLzma();
	QLzma(const QString& in);
	QLzma(const QString& in, const QString& out);
	~QLzma();

	int compressData(const unsigned char* data, size_t len, unsigned char *outBuf, size_t* destLen, int level=7, unsigned int dictSize=1 << 16 /*64kb*/);//char* data_out);
	void compress();
	//void extract();

	size_t inSize() const;
	size_t unpackSize() const;
/*!
  can be stdin, stdout
*/
	void setInPath(const QString& in);//list?
	void setOutPath(const QString& out);
	void setLevel(int level);

	QString outPath();

private:
	QLzmaPrivate *d;
};

#endif // QLZMA_H
