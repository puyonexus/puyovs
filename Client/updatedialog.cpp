#include "updatedialog.h"
#include "ui_updatedialog.h"
#include "pvsapplication.h"
#include "updaterudb.h"
#include "common.h"

#include <qglobal.h>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QTimer>
#include <QRegExp>
#include <QProcess>

#include <stdlib.h>

#if defined(Q_OS_WIN)

bool isElevated()
{
    BOOL elevated = FALSE;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            elevated = Elevation.TokenIsElevated;
        }
    }

    if (hToken) {
        CloseHandle(hToken);
    }

    return elevated == TRUE;
}

bool elevateProcess()
{
    wchar_t szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
        return false;
    }

    SHELLEXECUTEINFO sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = szPath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteEx(&sei)) {
        DWORD dwError = GetLastError();
        if (dwError == ERROR_CANCELLED) {
            return false;
        }
    }

    _exit(0);
}

#elif defined(Q_OS_MAC)

bool isElevated()
{
    return geteuid() == 0;
}

bool elevateProcess()
{
    QString executablePath = QCoreApplication::applicationFilePath();
    QString commandLine = "osascript -e 'do shell script \"";
    commandLine += executablePath;
    commandLine += " --update-only\" with prompt \"Puyo VS needs to update.\" with administrator privileges'";

    QProcess process;
    process.start(commandLine);
    process.waitForFinished();
    return true;
}

#elif defined(Q_OS_LINUX)

bool isElevated()
{
    return geteuid() == 0;
}

bool elevateProcess()
{
    QString executablePath = QCoreApplication::applicationFilePath();
    QString commandLine = "gksu -- \"";
    commandLine += executablePath;
    commandLine += "\" --update-only";

    QProcess process;
    process.start(commandLine);
    process.waitForFinished();
    return true;
}

#else

bool isElevated()
{
    return false;
}

bool elevateProcess()
{
    return false;
}

#endif

UpdateDialog::UpdateDialog(QString binPath, QWidget *parent) :
    QDialog(parent),
    binPath(binPath),
    ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);

    // open the ldb
    QFile ldbFile("update.ldb");
    if(!ldbFile.open(QFile::ReadOnly))
        qDebug() << "Could not open ldb.";
    else if(!ldb.read(&ldbFile))
        qDebug() << "Could not read ldb.";
    else qDebug() << "Read ldb file.";

    // update root
    updateUrl = "https://upd.puyovs.com";

    // create a network access manager
    netMan = new QNetworkAccessManager(this);

    // download the udb
    QNetworkRequest request;
    request.setUrl(fullUrl("update.udb"));
    request.setRawHeader("User-Agent", PUYOVS_USER_AGENT);
    QNetworkReply *reply = netMan->get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(taskProgress(qint64,qint64)));
    connect(reply, SIGNAL(readyRead()), SLOT(taskReadyRead()));
    connect(reply, SIGNAL(finished()), SLOT(beginUpdate()));
    taskDevice = reply;
    ui->overallProgress->setVisible(false);
}

UpdateDialog::~UpdateDialog()
{
    writeLDB();
    delete ui;
}

QUrl UpdateDialog::fullUrl(QString updatePath)
{
    return QUrl(QString().append(updateUrl).append('/').append(updatePath));
}

void UpdateDialog::writeLDB()
{
    QFile ldbFile("update.ldb");
    ldbFile.open(QFile::WriteOnly);
    if(!ldb.write(&ldbFile))
        QMessageBox::critical(this,
                              tr("Updater"),
                              tr("Could not write to revisions database...\n"
                                 "Update failed."));
    ldbFile.flush();
    ldbFile.close();
    qDebug() << "Wrote ldb file.";
}

void UpdateDialog::taskProgress(qint64 curr, qint64 max)
{
    ui->taskProgress->setRange(0, max);
    ui->taskProgress->setValue(curr);
}

void UpdateDialog::taskReadyRead()
{
    taskBuffer.append(taskDevice->read(taskDevice->bytesAvailable()));
}

void UpdateDialog::beginUpdate()
{
    UpdaterUDB udb;
    {
        QBuffer udbBuffer(&taskBuffer);
        udbBuffer.open(QBuffer::ReadOnly);
        bool success = udb.read(&udbBuffer);
        if (!success)
        {
            QTimer::singleShot(500, this, SLOT(close()));
            ui->taskProgress->setFormat("Error reading update.udb");
            return;
        }
    }
    taskBuffer.clear();

    foreach(const UpdaterFile &file, udb.files)
    {
        QString filename = file.filename();
		QString localFn = file.localFilename();
		QString platform = file.platform();
        qint32 version = file.version();
		if(!platform.isEmpty() && platform != platformStr)
			continue;

        if(file.isFolder())
        {
			QDir().mkpath(localFn);
        }
        else if(version != ldb.version(localFn))
		{
            DownloaderTask task;
			task.url = fullUrl(filename);
            if (!platform.isEmpty()) {
                task.target = QDir(binPath).filePath(localFn);
            } else {
                task.target = localFn;
            }
            task.revision = version;

            qDebug() << "Queueing download of" << filename << "at revision" << version << "(have " << ldb.version(localFn) << ")";
            downloadQueue.enqueue(task);
        }
    }

    if(downloadQueue.isEmpty())
    {
        QTimer::singleShot(500, this, SLOT(close()));
        ui->taskProgress->setFormat("Up to date.");
        return;
    }

    ui->overallProgress->setVisible(true);
    ui->overallProgress->setRange(0, downloadQueue.length());
    ui->overallProgress->setFormat(tr("Getting update... %p%"));

    nextTask();
}

void UpdateDialog::nextTask()
{
    if(downloadQueue.isEmpty())
    {
        QTimer::singleShot(500, this, SLOT(close()));
        ui->overallProgress->setFormat("Download complete.");
        ui->taskProgress->hide();

		if(removeQueue.isEmpty())
        {
            return;
        }
        else
        {
            QStringList arguments;
			arguments << "--remove-cruft";
			foreach(RemoveTask task, removeQueue)
				arguments << task.file;

			QProcess::startDetached(
				QFileInfo(platformBinary).absoluteFilePath(), arguments);

            writeLDB();
			_exit(0);
        }
    }

    task = downloadQueue.dequeue();
	ui->taskProgress->setFormat(tr("Downloading %1... %p%").arg(task.target));
    qDebug() << "File: " << task.url << "; Target: " << task.target;
    QNetworkRequest request;
    request.setUrl(task.url);
    request.setRawHeader("User-Agent", PUYOVS_USER_AGENT);
    QNetworkReply *reply = netMan->get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(taskProgress(qint64,qint64)));
    connect(reply, SIGNAL(readyRead()), SLOT(taskReadyRead()));
    connect(reply, SIGNAL(finished()), SLOT(taskDone()));
    taskDevice = reply;
}

void UpdateDialog::taskDone()
{
	QString targetOld = task.target + ".old";
    ui->overallProgress->setValue(ui->overallProgress->value() + 1);

	// Move old file
	QFile::remove(targetOld);
	while(QFile::exists(task.target) && !QFile::rename(task.target, targetOld)) {
        if (isElevated()) {
            if(QMessageBox::warning(this, tr("Updater"),
                                    tr("Error moving %1 out of the way...\n"
                                    "Please make sure there are no other "
                                    "instances of Puyo VS running.").arg(task.target),
                                    QMessageBox::Retry, QMessageBox::Abort) == QMessageBox::Abort) {
                close();
                return;
            }
        } else {
            if (!elevateProcess()) {
                QMessageBox::warning(this, tr("Updater"),
                                     tr("Error elevating process...\n"
                                        "The updater will now exit."));
            }
            close();
            return;
        }
    }

    // Save to disk
	QFile saveTo(task.target);
    if(!saveTo.open(QFile::WriteOnly)) {
        if (isElevated()) {
            writeError(task.target);
        } else {
            if (!elevateProcess()) {
                QMessageBox::warning(this, tr("Updater"),
                                     tr("Error elevating process...\n"
                                        "The updater will now exit."));
            }
        }
        close();
        return;
    }
    saveTo.write(taskBuffer);
    saveTo.flush();
    saveTo.close();
    taskBuffer.clear();
	ldb.setVersion(task.target, task.revision);

	// Remove old file, if possible.
	if(QFile::exists(targetOld) && !QFile::remove(targetOld))
    {
        // Windows hates us.
		RemoveTask rmtask;
		rmtask.file = targetOld;
		removeQueue.enqueue(rmtask);
	}

    nextTask();
}

void UpdateDialog::writeError(QString target)
{
    QMessageBox::critical(this,
                          tr("Updater"),
                          tr("An error occured writing to the file %1.\n"
                             "Updater will now exit.").arg(target));

    close();
}
