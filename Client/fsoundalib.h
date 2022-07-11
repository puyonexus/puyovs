#pragma once

#include <QObject>
#include "../Puyolib/frontend.h"

class GameAudio;

class FSoundAlib : public ppvs::fsound, public QObject
{
public:
	FSoundAlib(const QString& fn, GameAudio* audio, QObject* parent);
	~FSoundAlib() override;

	void play() override;
	void stop() override;

private:
	QString fn;
	GameAudio* audio;
	bool error;
};
