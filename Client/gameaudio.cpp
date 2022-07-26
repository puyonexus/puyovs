#include "gameaudio.h"
#include "common.h"
#include <alib/audiolib.h>
#include <alib/stream.h>

GameAudio::GameAudio(QObject* parent)
	: QObject(parent)
{
	init();
}

GameAudio::~GameAudio()
{
	delete audioDevice;
}

void GameAudio::play(const QString& path)
{
	if (sampleCache.contains(path)) {
		audioDevice->play(sampleCache[path]);
	} else {
		alib::Stream stm(path.toUtf8().data());
		sampleCache.insert(path, stm);
		audioDevice->play(stm);
	}
}

void GameAudio::init()
{
	audioDevice = alib::open();
}
