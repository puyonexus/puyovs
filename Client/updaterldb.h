#ifndef UPDATERLDB_H
#define UPDATERLDB_H

#include <qglobal.h>
#include <QMap>
class QIODevice;

class UpdaterLDB
{
public:
    bool read(QIODevice *stream);
    bool write(QIODevice *stream);

    qint32 version(QString fn);
    void setVersion(QString fn, int version);

    QMap<QString, quint32> fileVersions;
};

#endif // UPDATERLDB_H
