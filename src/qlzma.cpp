#include "qlzma.h"

#include <fstream>
#include <vector>
#include <assert.h>

#include <qfile.h>
#include <qfileinfo.h>
#include <qdatetime.h>

#include "lzma/C/Types.h"
#include "lzma/C/LzmaEnc.h"

#include "qtcompat.h"
#include "gui/ezprogressdialog.h"
#include "utils/qt_util.h"
#include "utils/convert.h"
#include "msgdef.h"

#define LZMA86_SIZE_OFFSET (1 + LZMA_PROPS_SIZE)
#define LZMA86_HEADER_SIZE (LZMA86_SIZE_OFFSET + 8)

/*!
SzAllocForLzma is another interface which gives LZMA library pointers to the memory allocation and deallocation functions. To just use standard malloc and free functions, you can copy this code:
*/
static void * AllocForLzma(void *p, size_t size) { return malloc(size); }
static void FreeForLzma(void *p, void *address) { free(address); }
static ISzAlloc SzAllocForLzma = { &AllocForLzma, &FreeForLzma };



class QLzmaPrivate {
	Q_DECLARE_PUBLIC(QLzma)
public:
	QLzmaPrivate(QLzma* q)
		:q_ptr(q),in_path(""),out_path(""),level(7)
		,totalSize(0),processedSize(0),extra_msg(QObject::tr("Calculating..."))
		,last_elapsed(1),elapsed(0),time_passed(0),pause(false),left(0),ratio(1.0)
		,progressCallBack(new ICompressProgress)
	{
		init();
	}

	QLzmaPrivate(QLzma* q, const QString& in)
		:q_ptr(q),out_path(in),level(7)
		,totalSize(0),processedSize(0),extra_msg(QObject::tr("Calculating..."))
		,last_elapsed(1),elapsed(0),time_passed(0),pause(false),left(0),ratio(1.0)
		,progressCallBack(new ICompressProgress)
	{
		setInPath(in);
		out_path.append(".lzma");
		init();
	}
	QLzmaPrivate(const QString &in, const QString& out)
		:q_ptr(0),out_path(out),level(7)
		,totalSize(0),processedSize(0),extra_msg(QObject::tr("Calculating..."))
		,last_elapsed(1),elapsed(0),time_passed(0),pause(false),left(0),ratio(1.0)
		,progressCallBack(new ICompressProgress)
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
		self = 0;
	}

	inline size_t size() const { return instance()->totalSize;}

	void setInPath(const QString& path) {
		in_path = QFileInfo(path).absoluteFilePath();
		totalSize = QFile(in_path).size();
		max_str=QString(" / %1").arg(size2str(totalSize));
		if (!progress) {
			progress = new EZProgressDialog(QObject::tr("Calculating..."));
			progress->addButton(QObject::tr("Hide"), 0, 1,Qt::AlignRight);
			progress->addButton(QObject::tr("Pause"), 1);
#if CONFIG_QT4
			progress->button(1)->setCheckable(true);
#else
			progress->button(1)->setToggleButton(true);
#endif //CONFIG_QT4
			QObject::connect(progress->button(0), SIGNAL(clicked()), progress, SLOT(hide())); //Hide the widget will be faster. not showMinimum
			QObject::connect(progress->button(1), SIGNAL(clicked()), q_ptr, SLOT(pauseOrResume()));
			QObject::connect(progress, SIGNAL(canceled()), q_ptr, SLOT(stop()));
			progress->setAutoClose(false);
			progress->setAutoReset(false);
			progress->show();
		}
		progress->setMaximum(totalSize);
	}

	void estimate() {
		if(!pause)
			elapsed = last_elapsed + time.elapsed();
		speed = processedSize/(1+elapsed)*1000; //>0
		left = (totalSize-processedSize)/(1+speed);
		ratio = 100.0 * (qreal)compressedSize/(qreal)processedSize;
	}

	void updateMessage() {
		out_msg = g_BaseMsg_Ratio(in_path, totalSize, QString("%1%").arg(ratio, 0, 'g', 3), processedSize, max_str);
		extra_msg = g_ExtraMsg_Detail(speed, elapsed, left);
	}

	//Lzma will not call ICompressProgress when finished.
	void showFinish() {
		processedSize = totalSize;
		compressedSize = QFile(out_path).size();
		estimate();
		updateMessage();
		progress->setValue(processedSize);
		progress->setLabelText(out_msg + extra_msg);
		qApp->processEvents();
	}

	static QLzmaPrivate* instance() {
		if (!self) {

		}
		return self;
	}
	static ICompressProgress g_ProgressCallback;// = { &OnProgress };
	static SRes OnProgress(void *p, UInt64 inSize, UInt64 outSize);

	QLzma *q_ptr;
	QString in_path, out_path;
	int level;

	QTime time;
	uint totalSize, processedSize, compressedSize, uncompressedSize;
	//uint interval;
	QString out_msg, extra_msg;
	QString max_str;
	uint last_elapsed, elapsed, speed; //ms
	int time_passed;
	volatile bool pause;
	qreal left;
	qreal ratio;
	int tid;

private:
	void init() {
		g_time_convert = msec2secstr;
		initTranslations();
		self = this;
		progressCallBack = &QLzmaPrivate::g_ProgressCallback;
	}

	static QLzmaPrivate* self;
	static EZProgressDialog *progress;
	ICompressProgress *progressCallBack;
};

QLzmaPrivate* QLzmaPrivate::self = 0;
EZProgressDialog* QLzmaPrivate::progress = 0; //DO NOT new. Because it is before qApp created;

SRes QLzmaPrivate::OnProgress(void *p, UInt64 inSize, UInt64 outSize)
{
	QLzmaPrivate* q = QLzmaPrivate::instance();
	q->compressedSize = outSize;
	q->processedSize = inSize;

	q->estimate();
	q->updateMessage();

	progress->setValue(inSize);
	progress->setLabelText(q->out_msg + q->extra_msg);
	qApp->processEvents();
	//fprintf(stdout,"\r Finished: %.2f Ratio: %.2f (out: %llu in: %llu)\n",(double)inSize/(double)outSize,(double)outSize/inSize,outSize, inSize);
	//printf("p=%d", *(int*)(p));
	//fflush(stdout);
	return SZ_OK;
}
ICompressProgress QLzmaPrivate::g_ProgressCallback  = { &OnProgress };




QLzma::QLzma()
	:d_ptr(new QLzmaPrivate(this))
{}

QLzma::QLzma(const QString &in)
	:d_ptr(new QLzmaPrivate(this, in))
{}

QLzma::QLzma(const QString &in, const QString &out)
	:d_ptr(new QLzmaPrivate(in, out))
{}

QLzma::~QLzma()
{
	Q_D(QLzma);
	if(d->instance()) {
		delete d;
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
	if(*destLen < LZMA86_HEADER_SIZE)
		return SZ_ERROR_OUTPUT_EOF;
	size_t propsSize = LZMA_PROPS_SIZE;
	size_t outSizeProcessed = *destLen - LZMA86_HEADER_SIZE;
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

	Q_D(QLzma);
	int curRes = LzmaEncode( outBuf+LZMA86_HEADER_SIZE/*(Byte*)&outBuf[LZMA_PROPS_SIZE]*/,
		&outSizeProcessed, (Byte*)data, len,
		&props, outBuf+1, &propsSize, props.writeEndMark,
		d->progressCallBack, &SzAllocForLzma, &SzAllocForLzma);

	assert(curRes == SZ_OK && propsSize == LZMA_PROPS_SIZE);
	outBuf[0]=0;

	*destLen = LZMA86_HEADER_SIZE + outSizeProcessed;
	return curRes;////propsSize+destLen; //for ram compression

}

void QLzma::compress()
{
	Q_D(QLzma);

	QFile in(d->in_path);
	if (!in.open(QIODevice::ReadOnly)) {
		qWarning("Failed to open %s: %s", qPrintable(d->in_path), qPrintable(in.errorString()));
		return;
	}
	size_t size = in.size();
	QByteArray in_buffer = in.readAll();
	in.close();

	//Byte *out_buffer=new Byte[size];
	QByteArray out_buffer(size, 0);
	size_t len_compressed = LZMA86_HEADER_SIZE + size + size / 3 + 128;

	d->time.start();

	compressData((Byte*)in_buffer.constData(), size, (Byte*)out_buffer.data(), &len_compressed, d->level);
	d->showFinish();
	emit finished();

	QFile out(d->out_path);
	if (!out.open(QIODevice::WriteOnly)) {
		qWarning("Failed to open %s: %s", qPrintable(d->out_path), qPrintable(out.errorString()));
		return;
	}
	//size_t len_write = out.write((const char*)&out_buffer[0], len_compressed);
	size_t len_write = out.write(out_buffer, len_compressed);
	if (len_write != len_compressed) {
		qWarning("Write %s error(%u/%u writed): %s", qPrintable(d->out_path), len_write, len_compressed, qPrintable(out.errorString()));
		return;
	}
	out.close();
	//delete [] out_buffer;
}

size_t QLzma::inSize() const
{
	Q_D(const QLzma);
	return d->size();
}

size_t QLzma::unpackSize() const
{
	Q_D(const QLzma);
	std::ifstream is;
	is.open(qPrintable(d->in_path), std::ios_base::binary | std::ios_base::in);
	is.seekg(0, std::ios_base::end);
	size_t unpackSize = is.tellg();

	if (unpackSize < LZMA86_HEADER_SIZE) {
		qWarning("SZ_ERROR_INPUT_EOF");
		is.close();
		return -1;
	}

	is.seekg(LZMA86_SIZE_OFFSET, std::ios_base::beg);
	char size_block[sizeof(UInt64)];
	is.read(size_block, sizeof(UInt64));
	for (unsigned i = 0; i < sizeof(UInt64); ++i)
		unpackSize += ((UInt64)size_block[i]) << (i<<3);
	is.close();
	return unpackSize;
}

void QLzma::setInPath(const QString &in)
{
	Q_D(QLzma);
	d->in_path = in;
}

void QLzma::setOutPath(const QString &out)
{
	Q_D(QLzma);
	d->out_path = out;
}

void QLzma::setLevel(int level)
{
	Q_D(QLzma);
	d->level = level;
}

QString QLzma::outPath()
{
	Q_D(QLzma);
	return d->out_path;
}

void QLzma::extract()
{

}

void QLzma::pauseOrResume()
{
	Q_D(QLzma);
	d->pause = !d->pause;
	if(!d->pause) {
		d->last_elapsed = d->elapsed;
		d->time.restart();
		resume();
	} else {
		pause();
	}
}

void QLzma::pause()
{
	Q_D(QLzma);
	while(d->pause) {
		QT_UTIL::qWait(100);
	}
}

void QLzma::resume()
{

}

void QLzma::stop()
{
	Q_D(QLzma);
	if (d->instance()) {
		delete d;
	}
	qApp->quit();
}

void QLzma::timerEvent(QTimerEvent *)
{

}
