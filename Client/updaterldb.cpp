#include "updaterldb.h"
#include <QDataStream>
#include <QIODevice>
#include <QMapIterator>

bool UpdaterLdb::read(QIODevice* stream)
{
	QDataStream dataStream(stream);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	quint32 numEntries;
	char magic[4];
	constexpr uint magicLen = sizeof(magic);

	// Magic number
	dataStream.readRawData(magic, magicLen);
	if (memcmp(magic, "LDBV", 4) != 0) {
		return false;
	}

	// Number of entries
	dataStream >> numEntries;

	while (numEntries--) {
		quint32 fnLength, fnVersion;
		char* filename = nullptr;

		dataStream >> fnLength;
		filename = static_cast<char*>(malloc(fnLength + 1));
		memset(filename, 0, fnLength + 1);
		dataStream.readRawData(filename, fnLength);
		dataStream >> fnVersion;

		m_fileVersions[QString::fromUtf8(filename)] = fnVersion;
		free(filename);
	}

	return true;
}

bool UpdaterLdb::write(QIODevice* stream) const
{
	QDataStream dataStream(stream);
	dataStream.setByteOrder(QDataStream::LittleEndian);

	// Magic number
	dataStream.writeRawData("LDBV", 4);

	// Number of entries
	dataStream << static_cast<quint32>(m_fileVersions.count());

	QMapIterator it(m_fileVersions);
	while (it.hasNext()) {
		it.next();
		QByteArray filename = it.key().toUtf8();
		dataStream << static_cast<quint32>(filename.size());
		dataStream.writeRawData(filename.constData(), filename.size());
		dataStream << it.value();
	}

	return true;
}

quint32 UpdaterLdb::version(const QString& fn) const
{
	return m_fileVersions.value(fn, -1);
}

void UpdaterLdb::setVersion(const QString& fn, const int version)
{
	m_fileVersions[fn] = version;
}
