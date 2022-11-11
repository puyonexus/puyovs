#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <alib/audiolib.h>
#include <alib/buffer.h>
#include <alib/stream.h>
#include <stdio.h>

#ifdef WIN32
#include <Windows.h>
void yield() { Sleep(5); }
#else
#include <unistd.h>
void yield() { usleep(10000); }
#endif

class HttpStream final : public alib::BinaryStream {
	QString m_url;
	QNetworkReply* m_reply = nullptr;

public:
	explicit HttpStream(const QString& url)
		: m_url(url)
	{
		const auto manager = new QNetworkAccessManager(qApp);
		QNetworkRequest request;
		request.setUrl(url);
		m_reply = manager->get(request);

		QEventLoop loop;
		QEventLoop::connect(m_reply, &QNetworkReply::readyRead, &loop, &QEventLoop::quit);
		loop.exec();
	}

	~HttpStream() override
	{
		delete m_reply;
	}

	HttpStream(const HttpStream&) = delete;
	HttpStream& operator=(const HttpStream&) = delete;
	HttpStream(HttpStream&&) = delete;
	HttpStream& operator=(HttpStream&&) = delete;

	int read(void* ptr, size_t size) override
	{
		QByteArray data;
		quint64 totalReceived = 0;

		while (totalReceived < size) {
			qint64 bRequest = qMin<qint64>(4096, static_cast<qint64>(size) - data.size());
			char buffer[4096] = { 0 };
			bRequest = m_reply->read(buffer, bRequest);
			data.append(buffer, static_cast<int>(bRequest));
			totalReceived += bRequest;

			qApp->sendPostedEvents();
			qApp->processEvents();
		}

		fprintf(stderr, "[HTTP] Read %u of %u.\n", data.size(), static_cast<unsigned int>(size));

		memcpy(ptr, data.data(), data.size());
		return data.size();
	}

	int readChar() override
	{
		char c;
		return m_reply->getChar(&c) ? static_cast<unsigned char>(c) : EOF;
	}

	int write(const void*, size_t) override { return 0; }
	bool seekable() override { return false; }
	bool seek(long, SeekOrigin) override { return false; }
	long tell() override { return 0; }
	bool eof() override { return m_reply->atEnd(); }
	bool hasEof() override { return true; }
	void rewind() override { m_reply->reset(); }
	bool error() override { return m_reply->error() != QNetworkReply::NoError; }
	long size() override { return static_cast<long>(m_reply->size()); }

    [[nodiscard]] std::string url() const override { return m_url.toStdString(); }
};

class BufferedStream final : public alib::BinaryStream {
    BinaryStream* m_stream = nullptr;
	alib::Buffer m_buffer;
	int m_i = 0;

public:
    explicit BufferedStream(BinaryStream* stm)
		: m_stream(stm)
	{
		prebuffer(0x20000);
	}

	~BufferedStream() override
	{
		delete m_stream;
	}

	BufferedStream(const BufferedStream&) = delete;
	BufferedStream& operator=(const BufferedStream&) = delete;
	BufferedStream(BufferedStream&&) = delete;
	BufferedStream& operator=(BufferedStream&&) = delete;

	int read(void* ptr, const size_t size) override
	{
		int bufferNeeded = static_cast<int>(size - (m_buffer.size() - m_i));

		while (bufferNeeded > 0) {
			char newBuffer[4096] = { 0 };
            const int bufferReceived = m_stream->read(newBuffer, 4096);
			m_buffer.append(newBuffer, bufferReceived);
			bufferNeeded -= bufferReceived;

			yield();
		}

        const int bufferRead = m_buffer.read(ptr, size, m_i);
		m_i += bufferRead;
		fprintf(stderr, "[Buffer] Read %u of %u.\n", bufferRead, static_cast<unsigned int>(size));
		return bufferRead;
	}

	void prebuffer(const size_t bufferNeeded)
	{
        const auto newBuffer = new char[bufferNeeded];
        const int bufferReceived = m_stream->read(newBuffer, bufferNeeded);
		m_buffer.append(newBuffer, bufferReceived);
		delete[] newBuffer;

		fprintf(stderr, "Bytes prebuffered: %u\n", static_cast<unsigned int>(m_buffer.size()));
	}

	int write(const void*, size_t) override { return 0; }
	bool seekable() override { return true; }
	bool seek(long to, SeekOrigin origin) override
	{
		switch (origin) {
		case SeekOrigin::Beginning:
			m_i = to;
			break;
		case SeekOrigin::Current:
			m_i += to;
			break;
		case SeekOrigin::End:
			if (size() > 0)
				m_i = size() + to;
			else
				m_i = static_cast<int>(m_buffer.size() + to);
			break;
		}
		return true;
	}
	long tell() override { return m_i; }
	bool eof() override { return false; }
	bool hasEof() override { return m_stream->hasEof(); }
	void rewind() override { m_i = 0; }
	bool error() override { return m_stream->error(); }
	long size() override { return 0; }
	char* readAll(int* len) override { return m_stream->readAll(len); }
	alib::Buffer readAll() override { return m_stream->readAll(); }
    [[nodiscard]] std::string url() const override { return m_stream->url(); }
};

void print_help()
{
	fprintf(stderr, "Audiotest: attempts to play a (f)ile/HTTP (s)tream using Audiolib.\nArguments:\n -f: local filename\n -s: a valid HTTP URL\n\n");
}

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);
	alib::Device* device = alib::open();
	if (argc==3) {
		if (!strcmp(argv[1],"-f")) {
			if (!QFile::exists(argv[2])) {
				print_help();
				return 2;
			}
			device->play(alib::Stream(argv[2]));
			while (1) {

				qApp->sendPostedEvents();
				qApp->processEvents();

				yield();
				}
			}
		else if (!strcmp(argv[1], "-s")) {
			BufferedStream *stream = new BufferedStream(new HttpStream(argv[2]));
			device->play(stream);
			while (1) {
				stream->prebuffer(4096);

				qApp->sendPostedEvents();
				qApp->processEvents();

				yield();
				}
			}
		else {
			print_help();
			return 1;
			}




	}
	else {
		print_help();
		return 1;
	}


}
