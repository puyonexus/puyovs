#pragma once

#include <QMap>
#include <qglobal.h>

class QIODevice;

class UpdaterLDB {
public:
	bool read(QIODevice* stream);
	bool write(QIODevice* stream) const;

	qint32 version(QString fn);
	void setVersion(QString fn, int version);

	QMap<QString, quint32> fileVersions;
};
