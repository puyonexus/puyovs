#pragma once

#include <QObject>

namespace alib {
class Stream;
}

class Playlist;
class MusicStreamObserver;

class MusicPlayer : public QObject {
	Q_OBJECT

	friend class MusicStreamObserver;

	struct Priv;
	Priv* p;

public:
	enum LoopMode {
		NoLoop,
		LoopAtLeastOnce,
		LoopSingle,
		LoopAll,
	};

	MusicPlayer(Playlist& playlist, QObject* parent = nullptr);
	~MusicPlayer() override;

	void setLoopMode(LoopMode mode) const;
	LoopMode loopMode() const;

	void playStream(const alib::Stream& stm) const;
	void setVolume(float volume) const;
	int numberOfTimesLooped() const;

public slots:
	void next() const;
	void previous() const;
	void play() const;
	void resume() const;
	void pause() const;
	void stop() const;
};
