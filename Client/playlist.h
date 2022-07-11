#pragma once

#include <QUrl>
#include <QList>
#include <QStringList>
#include <QAbstractItemModel>

class QNetworkAccessManager;
class QIODevice;

class UrlDispatcher
{
	static QNetworkAccessManager* manager;

public:
	static QUrl urlFromString(QString dir, QString path);
	static QIODevice* openUrl(QUrl url);
};

class CacheDB
{
	static CacheDB* instance;
	QList<unsigned> hashes;
	CacheDB();
	CacheDB(const CacheDB& other);
	~CacheDB();
public:
	static CacheDB& get();

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

	bool operator==(const PlaylistEntry& other) const { return urlHash == other.urlHash; }
	bool operator==(unsigned hash) const { return urlHash == hash; }

	bool isValid() const;
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
	~Playlist() override;

	PlaylistEntry* child(int n);
	PlaylistEntry* byHash(unsigned hash);
	int childCount() const;
	bool isEmpty() const;
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

		struct Priv; Priv* p;
public:
	PlaylistModel(Playlist* playlist, QObject* parent = nullptr);
	~PlaylistModel() override;

	QModelIndex index(int row, int column, const QModelIndex& parent) const override;
	QModelIndex parent(const QModelIndex& child) const override;
	int rowCount(const QModelIndex& parent) const override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QMap<int, QVariant> itemData(const QModelIndex& index) const override;
	bool setItemData(const QModelIndex& index, const QMap<int, QVariant>& roles) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

	Qt::DropActions supportedDropActions() const override;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	bool insertRows(int row, int count, const QModelIndex& parent) override;
	bool removeRows(int row, int count, const QModelIndex& parent) override;
	bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;

public slots:
	bool submit() override;

private slots:
	void notifyAboutToInsertRow(int index);
	void notifyRowInserted(int index);
	void notifyAboutToRemoveRow(int index);
	void notifyRowRemoved(int index);
	void notifyAboutToMoveRows(int count, int from, int to);
	void notifyRowsMoved(int count, int from, int to);
};
