#include "playlist.h"
#include "common.h"
#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QDataStream>
#include <QRegExp>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QMimeData>

QByteArray download(QString fn)
{
    QIODevice *file = UrlDispatcher::openUrl(UrlDispatcher::urlFromString(QString(), fn));

    QByteArray data;
    while(!file->atEnd())
    {
        char buffer[512] = { 0 };
        file->read(buffer, 512);
        data.append(buffer, 512);
    }

    delete file;

    return data;
}

/******************************************************************************
 * UrlDispatcher implementation
 */

QNetworkAccessManager *UrlDispatcher::manager = 0;

QUrl UrlDispatcher::urlFromString(QString dir, QString path)
{
    QRegExp scheme("^[a-zA-Z0-9]{2,10}://");
    if(dir.contains(scheme) || path.contains(scheme))
        return QUrl(dir).resolved(QUrl::fromUserInput(path));
    else
        return QUrl::fromLocalFile(dir).resolved(QUrl::fromLocalFile(path));
}

QIODevice *UrlDispatcher::openUrl(QUrl url)
{
    if(!manager)
        manager = new QNetworkAccessManager(qApp);

    QNetworkRequest request;
    request.setUrl(url);
    request.setRawHeader("User-Agent", PUYOVS_USER_AGENT);
    QIODevice *file = manager->get(request);

    if(file->inherits("QNetworkReply"))
    {
        QEventLoop loop;
        loop.connect(file, SIGNAL(finished()), SLOT(quit()));
        QTimer::singleShot(10 * 1000, &loop, SLOT(quit())); // 10 sec timeout
        loop.exec();
    }

    return file;
}

/******************************************************************************
 * CacheDB implementation
 */

CacheDB *CacheDB::instance = 0;

CacheDB::CacheDB()
{
    QFile cache("User/Music/cache.db");
    cache.open(QIODevice::ReadOnly);
    QDataStream dataStream(&cache);

    hashes.reserve(cache.size() / sizeof(unsigned));

    while(!dataStream.atEnd())
    {
        unsigned hash;
        dataStream >> hash;
        hashes.append(hash);
    }
}

CacheDB::~CacheDB()
{
    write();
}

CacheDB &CacheDB::get()
{
    if(!instance) instance = new CacheDB();
    return *instance;
}

bool CacheDB::registerHashCode(unsigned hash)
{
    if(hashes.contains(hash))
        return false;

    hashes.append(hash);
    return true;
}

void CacheDB::removeHashCode(unsigned hash)
{
    hashes.removeOne(hash);
}

void CacheDB::write()
{
    QFile cache("User/Music/cache.db");
    cache.open(QIODevice::WriteOnly);
    QDataStream dataStream(&cache);

    foreach(unsigned hash, hashes) {
        dataStream << hash;
    }
}

/******************************************************************************
 * Playlist implementation
 */

Playlist::Playlist()
{
}

Playlist::Playlist(QString fn)
{
    mFilename = fn;

    if(!QFile(fn).exists())
        return;

    QByteArray data = download(fn);
    QList<QByteArray> entries = data.split('\n');

    QString wd = QFileInfo(fn).dir().absolutePath() + "/";

    foreach(QByteArray entry, entries)
    {
        QString fn = QString::fromUtf8(entry.data());
        fn = fn.trimmed();
        if(fn.size() < 3)
            continue;

        PlaylistEntry ple(wd, fn);

        if(mChildren.contains(ple))
            continue;

        mChildren.append(ple);
    }
}

Playlist::~Playlist()
{
    write();
}

PlaylistEntry *Playlist::child(int n)
{
    if(n < 0 || n >= mChildren.size())
        return 0;

    return &mChildren[n];
}

PlaylistEntry *Playlist::byHash(unsigned hash)
{
    for(int i = 0; i < mChildren.size(); ++i)
        if(mChildren[i] == hash)
            return &mChildren[i];

    return 0;
}

int Playlist::childCount()
{
    return mChildren.count();
}

bool Playlist::isEmpty()
{
    return childCount() < 1;
}

void Playlist::discover(QString dir)
{
    // Here, we add newly discovered files to the playlist automagically.
    if(mDiscoverPath.isEmpty())
    {
        mDiscoverPath = dir;

        QFileSystemWatcher *watcher = new QFileSystemWatcher(this);
        watcher->addPath(dir);

        connect(watcher, SIGNAL(directoryChanged(QString)), SLOT(discover(QString)));
    }

    QDir discoverDir(dir);

    CacheDB &db = CacheDB::get();

    foreach(QString name, discoverDir.entryList(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot | QDir::NoSymLinks))
    {
        PlaylistEntry entry("", QFileInfo(discoverDir.absoluteFilePath(name)).canonicalFilePath());

        if(!mChildren.contains(entry) && db.registerHashCode(entry.urlHash))
            mChildren.append(entry);
    }

    foreach(PlaylistEntry entry, mChildren)
    {
        QFileInfo fileInfo(entry.url.toLocalFile());

        if(!fileInfo.exists())
        {
            mChildren.removeOne(entry);
            db.removeHashCode(entry.urlHash);
        }
    }

    // Write db and playlist at the same time.
    db.write();
    write();
}

void Playlist::write()
{
    if(mFilename.isEmpty())
        return;

    QFile file(mFilename);
    file.open(QIODevice::WriteOnly);

    foreach(PlaylistEntry entry, mChildren)
        file.write((entry.toString() + "\r\n").toUtf8());

    file.close();
}

void Playlist::insert(PlaylistEntry entry, int before)
{
    emit aboutToInsertRow(before);
    mChildren.insert(before, entry);
    emit rowInserted(before);
}

void Playlist::add(PlaylistEntry entry)
{
    int lastidx = mChildren.size();
    emit aboutToInsertRow(lastidx);
    mChildren.append(entry);
    emit rowInserted(lastidx);
}

void Playlist::remove(int id)
{
    emit aboutToRemoveRow(id);
    mChildren.removeAt(id);
    emit rowRemoved(id);
}

PlaylistEntry Playlist::take(int id)
{
    PlaylistEntry entry = mChildren.at(id);
    remove(id);
    return entry;
}

void Playlist::move(int count, int from, int to)
{
    emit aboutToMoveRows(count, from, to);
    QList<PlaylistEntry> items;
    for(int i = 0; i < count; ++i)
        items.append(mChildren.takeAt(from));
    for(int i = 0; i < count; ++i)
        mChildren.insert(to + i, items[i]);
    emit rowsMoved(count, from, to);
}

/******************************************************************************
 * PlaylistEntry implementation
 */

PlaylistEntry::PlaylistEntry()
{
    urlHash = 0;
}

PlaylistEntry::PlaylistEntry(QString dir, QString entry)
{
    urlHash = 0;

    if(entry.isEmpty())
        return;

    QStringList parts = entry.split(",$");
    url = UrlDispatcher::urlFromString(dir, parts.takeFirst());

    if(!parts.empty())
        params = parts.takeFirst().split(',');

    urlHash = qHash(toString());
}

bool PlaylistEntry::isValid()
{
    return url.isValid();
}

QString PlaylistEntry::toString()
{
    QString str;

    if(!isValid())
        return str;

    if(url.isLocalFile())
        str = url.toLocalFile();
    else
        str = url.toString();

    if(params.count() > 0)
        str += ",$" + params.join(",");

    return str;
}


/******************************************************************************
 * PlaylistModel implementation
 */

struct PlaylistModel::Priv
{
    Playlist *playlist;
};

PlaylistModel::PlaylistModel(Playlist *playlist, QObject *parent)
    : QAbstractItemModel(parent), p(new Priv)
{
    p->playlist = playlist;

    // hook up notifiers
    connect(p->playlist, SIGNAL(aboutToInsertRow(int)),
            SLOT(notifyAboutToInsertRow(int)));
    connect(p->playlist, SIGNAL(rowInserted(int)),
            SLOT(notifyRowInserted(int)));
    connect(p->playlist, SIGNAL(aboutToRemoveRow(int)),
            SLOT(notifyAboutToRemoveRow(int)));
    connect(p->playlist, SIGNAL(rowRemoved(int)),
            SLOT(notifyRowRemoved(int)));
    connect(p->playlist, SIGNAL(aboutToMoveRows(int,int,int)),
            SLOT(notifyAboutToMoveRows(int,int,int)));
    connect(p->playlist, SIGNAL(rowsMoved(int,int,int)),
            SLOT(notifyRowsMoved(int,int,int)));
}

PlaylistModel::~PlaylistModel()
{
    delete p;
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid() || column != 0 || row < 0 || row >= p->playlist->childCount())
        return QModelIndex();

    PlaylistEntry *entry = p->playlist->child(row);
    return createIndex(row, column, qHash(entry->toString()));
}

QModelIndex PlaylistModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid()) return 0;

    return p->playlist->childCount();
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    PlaylistEntry *entry = p->playlist->byHash(index.internalId());

    if(!entry)
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
        // TODO: perhaps load into alib and get metadata?
        // do not need alib driver loaded even!
        return QFileInfo(entry->url.toLocalFile()).fileName();
    case Qt::EditRole:
        return entry->toString();
    default:
        return QVariant();
    }
}

bool PlaylistModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    PlaylistEntry *entry = p->playlist->byHash(index.internalId());

    if(!entry)
        return false;

    switch(role)
    {
    case Qt::EditRole:
        *entry = PlaylistEntry("", value.toString());
        return true;
    default:
        return false;
    }
}

QMap<int, QVariant> PlaylistModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> datas;
    datas[Qt::DisplayRole] = data(index, Qt::DisplayRole);
    datas[Qt::EditRole] = data(index, Qt::EditRole);
    return datas;
}

bool PlaylistModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    PlaylistEntry *entry = p->playlist->byHash(index.internalId());

    if(!entry)
        return false;

    if(!roles.contains(Qt::EditRole))
        return false;

    *entry = PlaylistEntry("", roles[Qt::EditRole].toString());

    return true;
}

Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::ItemIsDropEnabled;

    return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction | Qt::TargetMoveAction;
}

bool PlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if(data->hasUrls())
    {
        QList<QUrl> urlList;

        urlList = data->urls();

        foreach(QUrl url, urlList)
            p->playlist->add(PlaylistEntry("", url.toString()));

        return true;
    }
    else return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool PlaylistModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid())
        return false;

    for(int i = 0; i < count; ++i)
        p->playlist->insert(PlaylistEntry(), row);

    return true;
}

bool PlaylistModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(parent.isValid())
        return false;

    for(int i = 0; i < count; ++i)
        p->playlist->remove(row);

    return true;
}

bool PlaylistModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    if(sourceParent.isValid() || destinationParent.isValid())
        return false;

    p->playlist->move(count, sourceRow, destinationChild);

    return true;
}

bool PlaylistModel::submit()
{
    p->playlist->write();

    return true;
}

void PlaylistModel::notifyAboutToInsertRow(int index)
{
    emit beginInsertRows(QModelIndex(), index, index);
}

void PlaylistModel::notifyRowInserted(int)
{
    emit endInsertRows();
}

void PlaylistModel::notifyAboutToRemoveRow(int index)
{
    emit beginRemoveRows(QModelIndex(), index, index);
}

void PlaylistModel::notifyRowRemoved(int)
{
    emit endRemoveRows();
}

void PlaylistModel::notifyAboutToMoveRows(int count, int from, int to)
{
    emit beginMoveRows(QModelIndex(), from, from + count - 1, QModelIndex(), to);
}

void PlaylistModel::notifyRowsMoved(int, int, int)
{
    emit endMoveRows();
}
