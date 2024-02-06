#pragma once

#include "../Puyolib/Frontend.h"
#include <QObject>

class GameAudio;

class FSoundAlib : public ppvs::FeSound, public QObject {
public:
	FSoundAlib(const QString& fn, GameAudio* audio, QObject* parent);
	~FSoundAlib() override;

	void play() override;
	void stop() override;

	bool error() override;

private:
	QString fn;
	GameAudio* audio;
	bool m_error;
};
