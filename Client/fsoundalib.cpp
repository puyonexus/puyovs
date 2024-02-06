#include "fsoundalib.h"
#include "gameaudio.h"

FSoundAlib::FSoundAlib(const QString& fn, GameAudio* audio, QObject* parent)
	: QObject(parent)
	, fn(fn)
	, audio(audio)
	, m_error(false)
{
}

FSoundAlib::~FSoundAlib()
{
}

void FSoundAlib::play()
{
	if (m_error)
		return;

	audio->play(fn);
}

void FSoundAlib::stop()
{
}

bool FSoundAlib::error() {
	return m_error;
}
