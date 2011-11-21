#ifndef QLZMA_H
#define QLZMA_H

#include <qobject.h>


class QLzmaPrivate;
class QLzma : public QObject
{
	Q_OBJECT
public:
	QLzma();
	QLzma(const QString& in);
	QLzma(const QString& in, const QString& out);
	~QLzma();

	int compressData(const unsigned char* data, size_t len, unsigned char *outBuf, size_t* destLen, int level=7, unsigned int dictSize=1 << 16 /*64kb*/);//char* data_out);
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

signals:
	void finished();

public slots:
	void compress();
	void extract();
	void pauseOrResume();
	void pause();
	void resume();
	void stop();

protected:
	virtual void timerEvent(QTimerEvent *);

	Q_DECLARE_PRIVATE(QLzma)
	QLzmaPrivate *d_ptr;
};

#endif // QLZMA_H
