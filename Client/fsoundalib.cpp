#include "fsoundalib.h"
#include "gameaudio.h"
#include <alib/mixer.h>

FSoundAlib::FSoundAlib(const QString &fn, GameAudio *audio, QObject *parent)
    : QObject(parent), fn(fn), audio(audio), error(false)
{
}

FSoundAlib::~FSoundAlib()
{
}

void FSoundAlib::play()
{
    if(error)
        return;

    /*alib::Stream strm(fn.toUtf8().data());

    if(strm.error())
        error = true;

    mixer.play(strm);*/

    audio->play(fn);
}

void FSoundAlib::stop()
{
}
