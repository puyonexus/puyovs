#pragma once

#include "updaterldb.h"
class QIODevice;

enum class UdbFileFlags : quint16 {
	Directory = 0x1, // Is a directory.
	CreateOnlyOnce = 0x2, // Creates a file only the first time it is updated.
	CreateOnlyOnFirstRun = 0x4, // Creates a file when the db is being generated.
	CreateOnlyIfNewer = 0x8, // Creates a file only if a new revision is available.
	DeleteOnce = 0x10, // Causes a file to be deleted upon updating.
	DeleteAlways = 0x20, // Causes a file to be deleted every run.
	RemoveFromDb = 0x40, // Causes a file to be removed from ldb.
};

inline UdbFileFlags operator|(UdbFileFlags lhs, UdbFileFlags rhs)
{
	return static_cast<UdbFileFlags>(static_cast<quint16>(lhs) | static_cast<quint16>(rhs));
}

inline bool operator&(UdbFileFlags lhs, UdbFileFlags rhs)
{
	return static_cast<quint16>(lhs) & static_cast<quint16>(rhs);
}

// STRUCTURES
struct UdbHeader {
	quint32 magicNumber;
	quint16 version; // If the updater is too old it should update the launcher blindly
	quint16 revision;
	quint16 numFiles;
	quint16 reserved1;
	quint32 reserved2;
	char message[512];
};

struct UdbFileInfo {
	UdbFileFlags flags;
	quint16 revision;
	quint32 reserved;
	quint32 crc;
	quint32 nameLen;
};

class UpdaterFile {
public:
	bool read(QIODevice* stream);
    [[nodiscard]] qint32 version() const;
    [[nodiscard]] QString filename() const;
    [[nodiscard]] QString localFilename() const;
    [[nodiscard]] QString platform() const;

    [[nodiscard]] bool isFolder() const;

private:
	QString m_filename;
	QString m_localFilename;
	UdbFileInfo m_header {};
};

typedef QList<UpdaterFile> UpdaterFiles;

class UpdaterUdb {
public:
	bool read(QIODevice* stream);

	UdbHeader m_header {};
	UpdaterFiles m_files {};
};
