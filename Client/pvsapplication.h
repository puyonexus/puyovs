#pragma once

#include "settings.h"
#include <QApplication>

class MusicPlayer;
class Playlist;

class PVSApplication : public QApplication {
	Q_OBJECT

	struct Priv;
	Priv* p;

public:
	PVSApplication(int& argc, char** argv);
	~PVSApplication() override;

	// Settings
	Settings& settings() const;

	// Music
	enum MusicMode { MusicOff,
		MusicPause,
		MusicNormal,
		MusicFever };
	void setMusicMode(MusicMode mode, bool advance = false) const;
	void setMusicVolume(float volume, bool fever = false) const;

	MusicPlayer& musicPlayer() const;
	MusicPlayer& feverMusicPlayer() const;

	Playlist& playlist() const;
	Playlist& feverPlaylist() const;

private slots:
	void reloadSettings() const;
};

extern PVSApplication* pvsApp;
