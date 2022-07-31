#include "updaterudb.h"
#include <QIODevice>

bool fullRead(QIODevice* device, char* data, qint64 size)
{
	qint64 have = 0;
	while (have < size) {
		const qint64 got = device->read(data, size - have);
		if (got < 0) {
			return false;
		}
		have += got;
		if (device->atEnd()) {
			return false;
		}
		device->waitForReadyRead(1000);
	}

	return true;
}

bool UpdaterFile::read(QIODevice* stream)
{
	if (!fullRead(stream, reinterpret_cast<char*>(&m_header), sizeof m_header))
		return false;

	m_filename = QString::fromUtf8(stream->read(m_header.nameLen));

	return true;
}

qint32 UpdaterFile::version() const
{
	return m_header.revision;
}

QString UpdaterFile::filename() const
{
	return m_filename;
}

QString UpdaterFile::localFilename() const
{
	if (m_filename.startsWith("Platform-"))
		return m_filename.right(m_filename.length() - m_filename.indexOf('/', 1) - 1);
	return m_filename;
}

QString UpdaterFile::platform() const
{
	if (m_filename.startsWith("Platform-"))
		return m_filename.left(m_filename.indexOf('/', 1)).remove(0, 9);
	return {};
}

bool UpdaterFile::isFolder() const
{
	return m_header.flags & UdbFileFlags::Directory;
}

bool UpdaterUdb::read(QIODevice* stream)
{
	if (!fullRead(stream, reinterpret_cast<char*>(&m_header), sizeof m_header))
		return false;

	for (int i = 0; i < m_header.numFiles; ++i) {
		UpdaterFile file;
		if (file.read(stream))
			m_files.append(file);
		else
			return false;
	}

	return true;
}
