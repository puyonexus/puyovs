#include "updaterudb.h"
#include <QIODevice>

bool fullread(QIODevice* device, char* data, qint64 size)
{
	qint64 have = 0;
	while (have < size) {
		qint64 got = device->read(data, size - have);
		if (got < 0)
			return false;
		have += got;
		if (device->atEnd())
			return false;
		device->waitForReadyRead(1000);
	}

	return true;
}

bool UpdaterFile::read(QIODevice* stream)
{
	if (!fullread(stream, (char*)&header, sizeof header))
		return false;

	fname = QString::fromUtf8(stream->read(header.namelen));

	return true;
}

qint32 UpdaterFile::version() const
{
	return header.revision;
}

QString UpdaterFile::filename() const
{
	return fname;
}

QString UpdaterFile::localFilename() const
{
	if (fname.startsWith("Platform-"))
		return fname.right(fname.length() - fname.indexOf('/', 1) - 1);
	return fname;
}

QString UpdaterFile::platform() const
{
	if (fname.startsWith("Platform-"))
		return fname.left(fname.indexOf('/', 1)).remove(0, 9);
	return QString();
}

bool UpdaterFile::isFolder() const
{
	return header.flags & UDBDirectory;
}

bool UpdaterUDB::read(QIODevice* stream)
{
	if (!fullread(stream, (char*)&header, sizeof header))
		return false;

	for (int i = 0; i < header.numfiles; ++i) {
		UpdaterFile file;
		if (file.read(stream))
			files.append(file);
		else
			return false;
	}

	return true;
}
