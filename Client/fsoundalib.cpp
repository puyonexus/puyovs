#include "fsoundalib.h"
#include "gameaudio.h"

FSoundAlib::FSoundAlib(const QString& fn, GameAudio* audio, QObject* parent)
	: QObject(parent)
	, fn(fn)
	, audio(audio)
	, error(false)
{
}

FSoundAlib::~FSoundAlib()
{
}

void FSoundAlib::play()
{
	if (error)
		return;

	audio->play(fn);
}

void FSoundAlib::stop()
{
}
