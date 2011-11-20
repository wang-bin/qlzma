#include "qlzma.h"

#include <fstream>
#include <vector>
#include <assert.h>

#include <qfile.h>
#include <qstring.h>

#include "lzma/C/Types.h"
#include "lzma/C/LzmaEnc.h"
#include "global.h"


#define LZMA86_SIZE_OFFSET (1 + LZMA_PROPS_SIZE)
#define LZMA86_HEADER_SIZE (LZMA86_SIZE_OFFSET + 8)

/*!
SzAllocForLzma is another interface which gives LZMA library pointers to the memory allocation and deallocation functions. To just use standard malloc and free functions, you can copy this code:
*/
static void * AllocForLzma(void *p, size_t size) { return malloc(size); }
static void FreeForLzma(void *p, void *address) { free(address); }
static ISzAlloc SzAllocForLzma = { &AllocForLzma, &FreeForLzma };



class QLzmaPrivate {
public:
	QLzmaPrivate()
		:in_path(""),out_path(""),level(7),progressCallBack(new ICompressProgress)
	{
		init();
	}

	QLzmaPrivate(const QString& in)
		:out_path(in),level(7),progressCallBack(new ICompressProgress)
	{
		setInPath(in);
		out_path.append(".lzma");
		init();
	}
	QLzmaPrivate(const QString &in, const QString& out)
		:out_path(out),level(7),progressCallBack(new ICompressProgress)
	{
		setInPath(in);
		init();
	}
	~QLzmaPrivate() {
		if(progressCallBack) {
			delete progressCallBack;
			progressCallBack = 0;
		}
		if (progress) {
			delete progress;
			progress = 0;
		}
	}

	size_t size() const { return length;}
	void init() {
		progressCallBack = &QLzmaPrivate::g_ProgressCallback;
	}

	void setInPath(const QString& path) {
		in_path = path;
		length = QFile(in_path).size();
		if (!progress)
			progress = new QProgressDialog();
		progress->setMaximum(length);
	}

	static ICompressProgress g_ProgressCallback;// = { &OnProgress };
	static int length;
	static SRes OnProgress(void *p, UInt64 inSize, UInt64 outSize);

	QString in_path, out_path;
	int level;
	ICompressProgress *progressCallBack;
	static QProgressDialog *progress;
};


int QLzmaPrivate::length=0;
QProgressDialog* QLzmaPrivate::progress = 0; //DO NOT new. Because it is before qApp created;

SRes QLzmaPrivate::OnProgress(void *p, UInt64 inSize, UInt64 outSize)
{
	progress->setValue(inSize);
	progress->setLabelText(QObject::tr("Ratio")+QString(": %1%").arg((double)100.0*outSize/inSize,0,'g',3));
	qApp->processEvents();
	//fprintf(stdout,"\r Finished: %.2f Ratio: %.2f (out: %llu in: %llu)",(double)inSize/(double)length,(double)outSize/inSize,outSize, inSize);
	//fflush(stdout);
	//printf("p=%d", *(int*)(p));
	return SZ_OK;
}
ICompressProgress QLzmaPrivate::g_ProgressCallback  = { &OnProgress };




QLzma::QLzma()
	:d(new QLzmaPrivate)
{}

QLzma::QLzma(const QString &in)
	:d(new QLzmaPrivate(in))
{}

QLzma::QLzma(const QString &in, const QString &out)
	:d(new QLzmaPrivate(in,out))
{}

QLzma::~QLzma()
{
	if(d) {
		delete d;
		d = 0;
	}
}

/*!
lzma86 header (14 bytes):
  Offset Size  Description
	0     1    = 0 - no filter,
			   = 1 - x86 filter
	1     1    lc, lp and pb in encoded form
	2     4    dictSize (little endian)
	6     8    uncompressed size (little endian)
*/
int QLzma::compressData(const unsigned char *data, size_t len, unsigned char *outBuf, size_t* destLen, int level, unsigned int dictSize)//char* data_out)
{
	size_t propsSize = LZMA_PROPS_SIZE;
	size_t outSizeProcessed = *destLen - LZMA86_HEADER_SIZE;
	if(outSizeProcessed<0)
		return SZ_ERROR_OUTPUT_EOF;
	//std::vector<char> outBuf;
	//outBuf.resize(destLen);
	CLzmaEncProps props;
	LzmaEncProps_Init(&props);
	props.level = level;
	props.dictSize = dictSize;//1 << 16; // 64 KB
	//props.writeEndMark = 0; // 0 or 1

	//write size to header
	UInt64 t = len;
	for (int i = 0; i < 8; i++, t >>= 8) outBuf[LZMA86_SIZE_OFFSET + i] = (Byte)t;

	int curRes = LzmaEncode( outBuf+LZMA86_HEADER_SIZE/*(Byte*)&outBuf[LZMA_PROPS_SIZE]*/,
		&outSizeProcessed, (Byte*)data, len,
		&props, outBuf+1, &propsSize, props.writeEndMark,
		d->progressCallBack, &SzAllocForLzma, &SzAllocForLzma);

	assert(curRes == SZ_OK && propsSize == LZMA_PROPS_SIZE);
	outBuf[0]=0;

	*destLen=LZMA86_HEADER_SIZE+outSizeProcessed;
	return curRes;////propsSize+destLen; //for ram compression

}

void QLzma::compress()
{
	//QFile in_file(d->in_path);
	std::ifstream is;
	is.open(qstr2cstr(d->in_path),std::ios_base::binary |std::ios_base::in);
	is.seekg(0,std::ios_base::end);
	size_t size = is.tellg();
	is.seekg(0,std::ios_base::beg);

	char *buffer = new char[size];

	is.read(buffer,size);
	is.close();
	Byte *buffer_compressed=new Byte[size];
	size_t len_compressed = LZMA86_HEADER_SIZE+size + size / 3 + 128;
	compressData((Byte*)buffer,size,buffer_compressed,&len_compressed,d->level);
	std::ofstream os;
	os.open(qstr2cstr(d->out_path));
	os.write((const char*)&buffer_compressed[0],len_compressed);
	os.close();
	delete [] buffer_compressed;
}

size_t QLzma::inSize() const
{
	return d->size();
}

size_t QLzma::unpackSize() const
{
	std::ifstream is;
	is.open(qstr2cstr(d->in_path),std::ios_base::binary |std::ios_base::in);
	is.seekg(0,std::ios_base::end);
	size_t unpackSize = is.tellg();

	if (unpackSize < LZMA86_HEADER_SIZE) {
		ZDEBUG("SZ_ERROR_INPUT_EOF");
		is.close();
		return -1;
	}

	is.seekg(LZMA86_SIZE_OFFSET,std::ios_base::beg);
	char size_block[sizeof(UInt64)];
	is.read(size_block,sizeof(UInt64));
	for (unsigned i = 0; i < sizeof(UInt64); ++i)
		unpackSize += ((UInt64)size_block[i]) << (i<<3);
	is.close();
	return unpackSize;
}

void QLzma::setInPath(const QString &in)
{
	d->in_path=in;
}

void QLzma::setOutPath(const QString &out)
{
	d->out_path=out;
}

void QLzma::setLevel(int level)
{
	d->level=level;
}

QString QLzma::outPath()
{
	return d->out_path;
}
