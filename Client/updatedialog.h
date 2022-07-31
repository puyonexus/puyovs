#pragma once

#include "updaterldb.h"
#include <QByteArray>
#include <QDialog>
#include <QQueue>
#include <QUrl>
#include <qglobal.h>

#if defined(Q_OS_WIN)
static const QString platformBinary = "PuyoVS.exe";
static const QString platformStr = "win32";
#elif defined(Q_OS_LINUX)
#if __x86_64__
static const QString platformBinary = "puyovs";
static const QString platformStr = "linux-x86_64";
#elif __i386__
static const QString platformBinary = "puyovs";
static const QString platformStr = "linux-i386";
#elif __arm__
static const QString platformBinary = "puyovs";
static const QString platformStr = "linux-armhf";
#else
#error Unknown platform
#endif
#elif defined(Q_OS_MAC)
static const QString platformBinary = "PuyoVS";
static const QString platformStr = "macos";
#else
#error Unknown platform
#endif

class QNetworkAccessManager;
class QIODevice;

namespace Ui {
class UpdateDialog;
}

struct DownloaderTask {
	QUrl url;
	QString target;
	quint16 revision;
};
struct RemoveTask {
	QString file;
};

class UpdateDialog : public QDialog {
	Q_OBJECT

public:
	UpdateDialog(QString binPath, QWidget* parent = nullptr);
	~UpdateDialog() override;

	QUrl fullUrl(QString updatePath) const;
	void writeLDB();

private slots:
	void taskProgress(qint64 curr, qint64 max) const;
	void taskReadyRead();
	void beginUpdate();
	void nextTask();
	void taskDone();
	void writeError(QString target);

private:
	QString binPath;
	QString updateUrl;
	QQueue<DownloaderTask> downloadQueue;
	QQueue<RemoveTask> removeQueue;
	QIODevice* taskDevice;
	QByteArray taskBuffer;
	DownloaderTask task;
	Ui::UpdateDialog* ui;
	QNetworkAccessManager* netMan;
	UpdaterLdb ldb;
};
