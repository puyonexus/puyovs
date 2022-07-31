#include "musicplayer.h"
#include <QApplication>
#include <QEvent>
#include <QObject>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#include <qrandom.h>
#endif

#include "playlist.h"
#include "pvsapplication.h"
#include <alib/audiolib.h>
#include <alib/stream.h>

class MusicStreamObserver : public alib::StreamObserver {
public:
	MusicStreamObserver(MusicPlayer::Priv* mp)
		: mp(mp)
	{
	}

private:
	void reachedEnd() override;
	void reachedLoop() override;

	MusicPlayer::Priv* mp;
};

struct MusicPlayer::Priv : QObject {
	Playlist& playlist;
	int playlistPtr;
	MusicStreamObserver* stmObserver;
	alib::Device* device;
	alib::Stream currentStream;
	MusicPlayer::LoopMode loopMode;
	int looped;

	Priv(Playlist& p)
		: playlist(p)
		, looped(0)
	{
		stmObserver = new MusicStreamObserver(this);
		device = alib::open();
		playlistPtr = 0;
	}

	~Priv() override
	{
		alib::close();
		delete stmObserver;
	}

	bool playStream(const alib::Stream& stm)
	{
		if (currentStream != stm) {
			currentStream.stop();
			currentStream = stm;
		}

		currentStream.setObserver(stmObserver);
		return device->play(currentStream);
	}

	void setDeviceVolume(float volume) const
	{
		device->setVolume(volume);
	}

	void setCurrentStreamVolume(float volume)
	{
		currentStream.setVolume(volume);
	}

	void next()
	{
		if (playlist.childCount() == 0)
			return;

		looped = 0;
		if (!pvsApp->settings().boolean("music", "randomorder", true))
			playlistPtr++;
		else {
			// On Qt 5.10 and above, use QRandomGenerator instead.
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
			playlistPtr = int(int(QRandomGenerator::global()->generate()) * 1.0 / RAND_MAX * playlist.childCount());
#else
			playlistPtr = int(qrand() * 1.0 / RAND_MAX * playlist.childCount());
#endif
		}

		if (playlistPtr >= playlist.childCount())
			playlistPtr = 0;

		play();
	}

	void previous() const
	{
		if (playlist.childCount() == 0)
			return;
	}

	void play()
	{
		if (playlist.childCount() == 0)
			return;
		if (playlistPtr < 0)
			playlistPtr = playlist.childCount() - 1;
		if (playlistPtr >= playlist.childCount())
			playlistPtr = 0;

		if (!currentStream.error())
			currentStream.stop();

		// HACK: support qiodevice here.
		currentStream = alib::Stream(playlist.child(playlistPtr)->url.toLocalFile().toUtf8().data());
		currentStream.identifyAsMusic();
		if (!playStream(currentStream)) {
			next();
			currentStream.setVolume(1.0f);
		}
	}

	void resume()
	{
		if (currentStream.atEnd() || currentStream.error()) {
			next();
			currentStream.setVolume(1.0f);
		} else
			currentStream.resume();
	}

	void pause()
	{
		currentStream.pause();
	}

	void stop()
	{
		currentStream.stop();
		currentStream = alib::Stream();
	}

	void streamEnded()
	{
		if (currentStream.error())
			return;

		looped++;

		switch (loopMode) {
		case MusicPlayer::LoopMode::NoLoop:
			break;
		case MusicPlayer::LoopMode::LoopSingle:
			stop();
			play();
			break;
		case MusicPlayer::LoopMode::LoopAll:
			next();
			currentStream.setVolume(1.0f);
			break;
		}
	}
	void streamLooped()
	{
		looped++;
	}

protected:
	bool event(QEvent* e) override
	{
		if (e->type() == QEvent::User)
			streamEnded();
		else if (e->type() == static_cast<QEvent::Type>(QEvent::User + 1))
			streamLooped();

		return QObject::event(e);
	}
};

void MusicStreamObserver::reachedEnd()
{
	QEvent* uE = new QEvent(QEvent::User);
	qApp->postEvent(mp, uE);
}
void MusicStreamObserver::reachedLoop()
{
	QEvent* uE = new QEvent(static_cast<QEvent::Type>(QEvent::User + 1));
	qApp->postEvent(mp, uE);
}

MusicPlayer::MusicPlayer(Playlist& playlist, QObject* parent)
	: QObject(parent)
	, p(new Priv(playlist))
{
}

MusicPlayer::~MusicPlayer()
{
	delete p;
}

void MusicPlayer::setLoopMode(MusicPlayer::LoopMode mode) const
{
	p->loopMode = mode;
}

MusicPlayer::LoopMode MusicPlayer::loopMode() const
{
	return p->loopMode;
}

void MusicPlayer::playStream(const alib::Stream& stm) const
{
	p->playStream(stm);
}

void MusicPlayer::setVolume(float volume) const
{
	p->setCurrentStreamVolume(volume);
}

int MusicPlayer::numberOfTimesLooped() const
{
	return p->looped;
}

void MusicPlayer::next() const
{
	p->next();
}

void MusicPlayer::previous() const
{
	p->previous();
}

void MusicPlayer::play() const
{
	p->play();
}

void MusicPlayer::resume() const
{
	p->resume();
}

void MusicPlayer::pause() const
{
	p->pause();
}

void MusicPlayer::stop() const
{
	p->stop();
}
