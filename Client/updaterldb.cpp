#include <QIODevice>
#include <QDataStream>
#include <QMapIterator>
#include "updaterldb.h"

bool UpdaterLDB::read(QIODevice *stream)
{
    QDataStream dataStream(stream);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    quint32 numEntries;
    char magic[4];
    uint magicLen = 4;

    // magic number
    dataStream.readRawData(magic, magicLen);
    if(memcmp(magic, "LDBV", 4)) return false;

    // number of entries
    dataStream >> numEntries;

    while(numEntries--)
    {
        quint32 fnLength, fnVersion;
        char *filename = 0;

        dataStream >> fnLength;
        filename = (char *)malloc(fnLength + 1);
        memset(filename, 0, fnLength + 1);
        dataStream.readRawData(filename, fnLength);
        dataStream >> fnVersion;

        fileVersions[QString::fromUtf8(filename)] = fnVersion;
        free(filename);
    }

    return true;
}

bool UpdaterLDB::write(QIODevice *stream)
{
    QDataStream dataStream(stream);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    // magic number
    dataStream.writeRawData("LDBV", 4);

    // number of entries
    dataStream << quint32(fileVersions.count());

    QMapIterator<QString, quint32> it(fileVersions);
    while(it.hasNext())
    {
        it.next();
        QByteArray filename = it.key().toUtf8();
        dataStream << quint32(filename.size());
        dataStream.writeRawData(filename.constData(), filename.size());
        dataStream << quint32(it.value());
    }

    return true;
}

qint32 UpdaterLDB::version(QString fn)
{
    if(fileVersions.contains(fn))
        return fileVersions[fn];

    return -1;
}

void UpdaterLDB::setVersion(QString fn, int version)
{
    fileVersions[fn] = version;
}
