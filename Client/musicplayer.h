#pragma once

#include <QObject>

namespace alib { class Stream; }

class Playlist;
class MusicStreamObserver;
class MusicPlayer : public QObject
{
    Q_OBJECT

    friend class MusicStreamObserver;

    struct Priv; Priv *p;
public:
    enum LoopMode { NoLoop, LoopAtLeastOnce, LoopSingle, LoopAll };

    MusicPlayer(Playlist &playlist, QObject *parent = 0);
    ~MusicPlayer();

    void setLoopMode(LoopMode mode);
    LoopMode loopMode();

    void playStream(const alib::Stream &stm);
    void setVolume(float volume);
    int numberOfTimesLooped();

public slots:
    void next();
    void previous();
    void play();
    void resume();
    void pause();
    void stop();
};
