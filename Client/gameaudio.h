#ifndef GAMEAUDIO_H
#define GAMEAUDIO_H

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
    GameAudio(QObject *parent = 0);
    ~GameAudio();

    void play(const QString &path);

private slots:
    void init();

private:
    alib::Device *audioDevice;
    SampleCache sampleCache;
};

#endif // GAMEAUDIO_H
