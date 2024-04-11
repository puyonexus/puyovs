#pragma once

#include <QMap>
#include <QObject>
#include <QThread>
#include <alib/audiolib.h>
#include <alib/buffer.h>

typedef QMap<QString, alib::Stream> SampleCache;
typedef QMapIterator<QString, alib::Stream> SampleCacheIterator;

class GameAudio : public QObject {
	Q_OBJECT

public:
	GameAudio(QObject* parent = nullptr);
	~GameAudio() override;

	void play(const QString& path);
	bool test(const QString& path);

	bool error();

private slots:
	void init();

private:
	bool m_error;
	alib::Device* audioDevice;
	SampleCache sampleCache;
};
