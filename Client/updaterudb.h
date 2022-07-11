#pragma once

#include "updaterldb.h"
class QIODevice;

enum UDBFileFlags
{
	UDBDirectory = 0x1, // Is a directory.
	UDBCreateOnlyOnce = 0x2, // Creates a file only the first time it is updated.
	UDBCreateOnlyOnFirstRun = 0x4, // Creates a file when the db is being generated.
	UDBCreateOnlyIfNewer = 0x8, // Creates a file only if a new revision is available.
	UDBDeleteOnce = 0x10, // Causes a file to be deleted upon updating.
	UDBDeleteAlways = 0x20, // Causes a file to be deleted every run.
	UDBRemoveFromDb = 0x40, // Causes a file to be removed from versioning.
	UDBLastFlag = 0xFFFF, // Causes this enum to be 16-bit.
};

// STRUCTURES
struct UDBHeader
{
	quint32 magicnum;
	quint16 version; // if the updater is too old it should update the launcher blindly
	quint16 revision;
	quint16 numfiles;
	quint16 reserved1;
	quint32 reserved2;
	char message[512];
};

struct UDBFileInfo
{
	quint16 flags;
	quint16 revision;
	quint32 reserved;
	quint32 crc;
	quint32 namelen;
};

class UpdaterFile
{
public:
	bool read(QIODevice* stream);
	qint32 version() const;
	QString filename() const;
	QString localFilename() const;
	QString platform() const;

	bool isFolder() const;

private:
	QString fname;
	QString lfname;
	UDBFileInfo header;
};

typedef QList<UpdaterFile> UpdaterFiles;

class UpdaterUDB
{
public:
	bool read(QIODevice* stream);

	UDBHeader header;
	UpdaterFiles files;
};
