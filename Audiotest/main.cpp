#include <QCoreApplication>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <alib/audiolib.h>
#include <alib/buffer.h>
#include <alib/stream.h>

#ifdef WIN32
#include <windows.h>
void yield() { Sleep(5); }
#else
#include <unistd.h>
void yield() { usleep(10000); }
#endif

class HttpStream : public alib::BinaryStream {
	QString mUrl;
	QNetworkReply* reply;

public:
	HttpStream(QString url)
		: mUrl(url)
	{
		QNetworkAccessManager* manager = new QNetworkAccessManager(qApp);
		QNetworkRequest request;
		request.setUrl(url);
		reply = manager->get(request);

		QEventLoop loop;
		loop.connect(reply, SIGNAL(readyRead()), SLOT(quit()));
		loop.exec();
	}

	~HttpStream()
	{
		delete reply;
	}

	int read(void* ptr, size_t size)
	{
		QByteArray data;
		int totalReceived = 0;

		while (totalReceived < size) {
			int bRequest = static_cast<int>(qMin(size_t(4096), size - data.size()));
			char buffer[4096] = { 0 };
			bRequest = reply->read(buffer, bRequest);
			data.append(buffer, bRequest);
			totalReceived += bRequest;

			qApp->sendPostedEvents();
			qApp->processEvents();
		}

		fprintf(stderr, "[HTTP] Read %u of %u.\n", data.size(), static_cast<unsigned int>(size));

		memcpy(ptr, data.data(), data.size());
		return data.size();
	}

	int readChar()
	{
		char c;
		if (reply->getChar(&c))
			return (unsigned)c;
		else
			return EOF;
	}

	int write(const void*, size_t) { return 0; }
	bool seekable() { return false; }
	bool seek(long, SeekOrigin) { return false; }
	long tell() { return 0; }
	bool eof() { return reply->atEnd(); }
	virtual bool hasEof() { return true; }
	void rewind() { reply->reset(); }
	bool error() { return reply->error() != QNetworkReply::NoError; }
	long size() { return reply->size(); }

	std::string url() const { return mUrl.toStdString(); }
};

class BufferedStream : public alib::BinaryStream {
	alib::BinaryStream* stm;
	alib::Buffer buffer;
	int i;

public:
	BufferedStream(alib::BinaryStream* stm)
		: stm(stm)
	{
		i = 0;

		prebuffer(1024 * 64 * 2);
	}

	~BufferedStream()
	{
		delete stm;
	}

	int read(void* ptr, size_t size)
	{
		int bufferNeeded = static_cast<int>(size - (buffer.size() - i));

		while (bufferNeeded > 0) {
			char* newBuffer[4096] = { 0 };
			int bufferReceived = stm->read(newBuffer, 4096);
			buffer.append(newBuffer, bufferReceived);
			bufferNeeded -= bufferReceived;

			yield();
		}

		int bufferRead = buffer.read(ptr, size, i);
		i += bufferRead;
		fprintf(stderr, "[Buffer] Read %u of %u.\n", bufferRead, static_cast<unsigned int>(size));
		return bufferRead;
	}

	void prebuffer(size_t bufferNeeded)
	{
		char* newBuffer = new char[bufferNeeded];
		int bufferReceived = stm->read(newBuffer, bufferNeeded);
		buffer.append(newBuffer, bufferReceived);
		delete[] newBuffer;

		fprintf(stderr, "Bytes prebuffered: %u\n", static_cast<unsigned int>(buffer.size()));
	}

	int write(const void*, size_t) { return 0; }
	bool seekable() { return true; }
	bool seek(long to, SeekOrigin origin)
	{
		switch (origin) {
		case Beginning:
			i = to;
			break;
		case Current:
			i += to;
			break;
		case End:
			if (size() > 0)
				i = size() + to;
			else
				i = static_cast<int>(buffer.size() + to);
			break;
		}
		return true;
	}
	long tell() { return i; }
	bool eof() { return false; }
	bool hasEof() { return stm->hasEof(); }
	void rewind() { i = 0; }
	bool error() { return stm->error(); }
	long size() { return 0; }
	char* readAll(int* len) { return stm->readAll(len); }
	alib::Buffer readAll() { return stm->readAll(); }
	std::string url() const { return stm->url(); }
};

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	alib::Device* device = alib::open();

	// BufferedStream *stream = new BufferedStream(new HttpStream("http://..."));

	device->play(alib::Stream("loop-test.logg"));

	while (1) {
		// stream->prebuffer(4096);

		qApp->sendPostedEvents();
		qApp->processEvents();

		yield();
	}
}
