// Puyo Puyo VS
// Copyright (C) 2009-2022 Hernan and the Puyo Puyo VS contributors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "main.h"
#include "pvsapplication.h"
#include "mainwindow.h"
#include "updatedialog.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QTextCodec>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif

void qSleep(int ms)
{
#ifdef Q_OS_WIN
	Sleep(uint(ms));
#else
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
	nanosleep(&ts, NULL);
#endif
}

int main(int argc, char* argv[])
{
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
#if QT_VERSION < 0x050000
	// UTF-8 is now default in Qt 5!
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	QApplication::setAttribute(Qt::AA_X11InitThreads);
#if QT_VERSION >= 0x050600
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
	QApplication::setApplicationName("PuyoVS");
	PVSApplication a(argc, argv);
	QStringList args = a.arguments();

#ifdef PUYOVS_PACKAGED
	QDir::setCurrent(defaultAssetPath);
	QDebug() << "Running in packaged mode. puyovs.conf will be ignored.";
#else
	QString applicationPath = QCoreApplication::applicationDirPath();
	QDir applicationDir(applicationPath);

	// Check if a file named puyovs.conf exists.
	QString binPath = applicationDir.absolutePath();
	QString assetsPath = defaultAssetPath;

	// If puyovs.conf exists, use it.
	if (applicationDir.exists("puyovs.conf")) {
		QDir::setCurrent(binPath);
		QSettings settings("puyovs.conf", QSettings::IniFormat);
		if (settings.contains("paths/bin")) {
			binPath = settings.value("paths/bin").toString();
		}
		if (settings.contains("paths/assets")) {
			assetsPath = settings.value("paths/assets").toString();
		}
	}

	if (!assetsPath.isEmpty()) {
		QDir::setCurrent(assetsPath);
	}

	qDebug() << "Bin Path: " << binPath;
	qDebug() << "Asset Path: " << assetsPath;
#endif

#ifdef PUYOVS_UPDATER_ENABLED
	if (!args.contains("--no-update")) {
		if (args.contains("--remove-cruft")) {
			qSleep(500);
			qDebug() << "Removing old files.";

			char** arg = &argv[2];
			while (*arg)
			{
				QString file = *(arg++);

				qDebug() << "Removing" << file;
				QFile::remove(file);
			}
		}
		else
		{
			UpdateDialog d(binPath);
			QDesktopWidget* desktop = QApplication::desktop();
			QRect screenGeom = desktop->screenGeometry(QCursor::pos());
			QRect frame = d.frameGeometry();
			frame.moveCenter(screenGeom.center());
			d.move(frame.topLeft());
			d.exec();
		}
	}
#endif

	if (args.contains("--update-only")) {
		return 0;
	}

	MainWindow* w = new MainWindow();
	w->showTelemetryPrompt();
	w->exec();
	return 0;
}
