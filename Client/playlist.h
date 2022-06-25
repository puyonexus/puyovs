#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QUrl>
#include <QList>
#include <QStringList>
#include <QAbstractItemModel>

class QNetworkAccessManager;
class QIODevice;

class UrlDispatcher
{
    static QNetworkAccessManager *manager;

public:
    static QUrl urlFromString(QString dir, QString path);
    static QIODevice *openUrl(QUrl url);
};

class CacheDB
{
    static CacheDB *instance;
    QList<unsigned> hashes;
    CacheDB();
    CacheDB(const CacheDB &other);
    ~CacheDB();
public:
    static CacheDB &get();

    // returns true if the hash code was inserted
    // otherwise, it was already there
    bool registerHashCode(unsigned hash);
    void removeHashCode(unsigned hash);

    void write();
};

class PlaylistEntry
{
public:
    PlaylistEntry();
    PlaylistEntry(QString dir, QString entry);

    bool operator==(const PlaylistEntry &other) const { return urlHash == other.urlHash; }
    bool operator==(unsigned hash) const { return urlHash == hash; }

    bool isValid();
    QString toString();

    QUrl url;
    unsigned urlHash;
    QStringList params;
};

class QFileSystemWatcher;
class Playlist : public QObject
{
    Q_OBJECT
public:
    Playlist();
    Playlist(QString fn);
    ~Playlist();

    PlaylistEntry *child(int n);
    PlaylistEntry *byHash(unsigned hash);
    int childCount();
    bool isEmpty();
    Q_SLOT void discover(QString dir);

    void write();
    void insert(PlaylistEntry entry, int before);
    void add(PlaylistEntry entry);
    void remove(int id);
    PlaylistEntry take(int id);
    void move(int count, int from, int to);

signals:
    void aboutToInsertRow(int index);
    void rowInserted(int index);

    void aboutToRemoveRow(int index);
    void rowRemoved(int index);

    void aboutToMoveRows(int count, int from, int to);
    void rowsMoved(int count, int from, int to);

private:
    QString mFilename, mDiscoverPath;
    QList<PlaylistEntry> mChildren;
};

class PlaylistModel : public QAbstractItemModel
{
    Q_OBJECT

    struct Priv; Priv *p;
public:
    PlaylistModel(Playlist *playlist, QObject *parent = 0);
    ~PlaylistModel();

    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QMap<int, QVariant> itemData(const QModelIndex &index) const;
    bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool insertRows(int row, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

public slots:
    bool submit();

private slots:
    void notifyAboutToInsertRow(int index);
    void notifyRowInserted(int index);
    void notifyAboutToRemoveRow(int index);
    void notifyRowRemoved(int index);
    void notifyAboutToMoveRows(int count, int from, int to);
    void notifyRowsMoved(int count, int from, int to);
};

#endif // PLAYLIST_H
