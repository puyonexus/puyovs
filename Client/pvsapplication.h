#ifndef PVSAPPLICATION_H
#define PVSAPPLICATION_H

#include <QApplication>
#include "settings.h"

class MusicPlayer;
class Playlist;

class PVSApplication : public QApplication
{
    Q_OBJECT

    struct Priv; Priv *p;
public:
    PVSApplication(int &argc, char **argv);
    ~PVSApplication();

    // Settings
    Settings &settings();

    // Music
    enum MusicMode { MusicOff, MusicPause, MusicNormal, MusicFever };
    void setMusicMode(MusicMode mode, bool advance = false);
    void setMusicVolume(float volume, bool fever=false);

    MusicPlayer &musicPlayer();
    MusicPlayer &feverMusicPlayer();

    Playlist &playlist();
    Playlist &feverPlaylist();

private slots:
    void reloadSettings();
};

extern PVSApplication *pvsApp;

#endif // PVSAPPLICATION_H
