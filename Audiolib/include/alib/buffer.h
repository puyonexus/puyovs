#ifndef BUFFER_H
#define BUFFER_H

#include "audiolib.h"
#include <string>

namespace alib {

class Buffer
{
    ALIB_DECLARE_IMPSHARED;

public:
    Buffer();
    /**
     * Creates new buffer from data.
     * Warning: this takes ownership of the buffer.
     */
    Buffer(char *data, size_t size);
    Buffer(const Buffer &other);
    ~Buffer();

    int read(void *buffer, size_t amount, long offset);
    void append(const void *data, size_t size);
    void removeFront(size_t size);
    void truncate(size_t size);
    void clear();
    bool isEmpty() const;

    const char *data() const;
    size_t size() const;

    char *lock(int *size);
    void unlock();
};

class BinaryStream
{
public:
    enum SeekOrigin
    {
        Beginning,
        Current,
        End
    };

    virtual ~BinaryStream();

    virtual int read(void *ptr, size_t size) = 0;
    virtual int readChar();
    virtual int write(const void *ptr, size_t size) = 0;

    virtual bool seekable() { return false; }
    virtual bool seek(long to, SeekOrigin origin) = 0;
    virtual long tell() = 0;
    virtual bool eof() = 0;
    virtual bool hasEof() = 0;
    virtual void rewind() = 0;
    virtual bool error() = 0;
    virtual long size();

    virtual char *readAll(int *len);
    virtual Buffer readAll();

    virtual std::string url() const;

    static BinaryStream *openUrl(const char *url, const char *mode = "rb");
};

class FileStream : public BinaryStream, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    FileStream(const char *fn, const char *mode);
    ~FileStream();

    int read(void *ptr, size_t size);
    int readChar();
    int write(const void *ptr, size_t size);

    bool seekable();
    bool seek(long to, SeekOrigin origin);
    long tell();
    bool eof();
    bool hasEof();
    void rewind();
    bool error();
    long size();

    char *readAll(int *len);
    Buffer readAll();

    std::string url() const;
};

class MemoryStream : public BinaryStream, NonCopyable
{
    ALIB_DECLARE_PRIV;

public:
    MemoryStream(Buffer &b);
    ~MemoryStream();

    int read(void *ptr, size_t size);
    int write(const void *ptr, size_t size);

    bool seekable();
    bool seek(long to, SeekOrigin origin);
    long tell();
    bool eof();
    bool hasEof();
    void rewind();
    bool error();
    long size();
};

}

#endif // BUFFER_H
