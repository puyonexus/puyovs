#include "pvsapplication.h"
#include "common.h"
#include "musicplayer.h"
#include "playlist.h"
#include <QDir>
#include <alib/audiolib.h>

PVSApplication* pvsApp = nullptr;

struct PVSApplication::Priv {
	Settings* settings;

	Playlist normalPlaylist, feverPlaylist;
	MusicPlayer normalPlayer, feverPlayer;

	Priv()
		: normalPlaylist("User/Music/Normal.m3u")
		, feverPlaylist("User/Music/Fever.m3u")
		, normalPlayer(normalPlaylist)
		, feverPlayer(feverPlaylist)
	{
		normalPlaylist.discover("User/Music/Normal");
		feverPlaylist.discover("User/Music/Fever");
	}
};

PVSApplication::PVSApplication(int& argc, char** argv)
	: QApplication(argc, argv)
	, p(new Priv)
{
	// Copy settings.json if necessary
	if (!QDir(getDataLocation()).exists()) {
		QDir().mkpath(getDataLocation());
	}
	if (QFile("Settings.json").exists() && !QFile(getDataLocation() + "/Settings.json").exists()) {
		QFile("Settings.json").copy(getDataLocation() + "/Settings.json");
	}

	p->settings = new Settings(this);

	// Make sure these exist.
	QDir().mkdir("User/Music");
	QDir().mkdir("User/Music/Fever");
	QDir().mkdir("User/Music/Normal");

	pvsApp = this;

	reloadSettings();
	connect(p->settings, SIGNAL(saved()), SLOT(reloadSettings()));
}

PVSApplication::~PVSApplication()
{
}

Settings& PVSApplication::settings() const
{
	return *p->settings;
}

void PVSApplication::setMusicMode(PVSApplication::MusicMode mode, bool advance) const
{
	if (!p->settings->boolean("launcher", "enablemusic", true)) {
		p->normalPlayer.stop();
		p->feverPlayer.stop();
		return;
	}

	switch (mode) {
	case MusicMode::MusicOff:
		p->normalPlayer.stop();
		p->feverPlayer.stop();
		break;
	case MusicMode::MusicPause:
		p->normalPlayer.pause();
		p->feverPlayer.pause();
		break;
	case MusicMode::MusicNormal:
		if (advance) {
			if (p->settings->boolean("music", "looponce", true)) {
				if (p->normalPlayer.numberOfTimesLooped() > 0)
					p->normalPlayer.next();
				else
					p->normalPlayer.resume();
			} else
				p->normalPlayer.next();
		} else
			p->normalPlayer.resume();

		p->feverPlayer.stop();
		break;
	case MusicMode::MusicFever:
		p->normalPlayer.pause();

		if (advance)
			p->feverPlayer.next();
		else
			p->feverPlayer.resume();
		break;
	}

	if (p->normalPlaylist.isEmpty())
		p->normalPlayer.stop();
	if (p->feverPlaylist.isEmpty())
		p->feverPlayer.stop();
}

void PVSApplication::setMusicVolume(float volume, bool fever) const
{
	if (!fever)
		p->normalPlayer.setVolume(volume);
	else
		p->feverPlayer.setVolume(volume);
}

MusicPlayer& PVSApplication::musicPlayer() const
{
	return p->normalPlayer;
}

MusicPlayer& PVSApplication::feverMusicPlayer() const
{
	return p->feverPlayer;
}

Playlist& PVSApplication::playlist() const
{
	return p->normalPlaylist;
}

Playlist& PVSApplication::feverPlaylist() const
{
	return p->feverPlaylist;
}

void PVSApplication::reloadSettings() const
{
	if (!p->settings->boolean("launcher", "enablemusic", true))
		setMusicMode(MusicMode::MusicOff);

	if (p->settings->integer("music", "loopmode", 0) == 0)
		p->normalPlayer.setLoopMode(MusicPlayer::LoopMode::LoopSingle);
	else
		p->normalPlayer.setLoopMode(MusicPlayer::LoopMode::LoopAll);

	alib::Device* device = alib::open();
	int soundVolume = qBound(qint64(0), p->settings->integer("music", "soundvolume", 100), qint64(100));
	int musicVolume = qBound(qint64(0), p->settings->integer("music", "musicvolume", 100), qint64(100));
	device->setSoundVolume(soundVolume / 100.0f);
	device->setMusicVolume(musicVolume / 100.0f);
}
