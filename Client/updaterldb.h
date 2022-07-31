#pragma once

#include <QMap>
#include <qglobal.h>

class QIODevice;

class UpdaterLdb {
public:
	bool read(QIODevice* stream);
	bool write(QIODevice* stream) const;

    [[nodiscard]] quint32 version(const QString& fn) const;
	void setVersion(const QString& fn, int version);

	QMap<QString, quint32> m_fileVersions;
};
