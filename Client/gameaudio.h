#pragma once

#include <alib/audiolib.h>
#include <alib/buffer.h>
#include <QThread>
#include <QObject>
#include <QMap>

typedef QMap<QString, alib::Stream> SampleCache;
typedef QMapIterator<QString, alib::Stream> SampleCacheIterator;

class GameAudio : public QObject
{
	Q_OBJECT

public:
	GameAudio(QObject* parent = nullptr);
	~GameAudio() override;

	void play(const QString& path);

private slots:
	void init();

private:
	alib::Device* audioDevice;
	SampleCache sampleCache;
};
