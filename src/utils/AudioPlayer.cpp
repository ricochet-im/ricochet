#include "AudioPlayer.h"
#include <QDebug>

//#define USE_QSOUNDEFFECT
#define USE_MEDIAPLAYER

#if defined(USE_QSOUNDEFFECT)

#include <QSoundEffect>

class AudioPlayer::Impl
{
public:
    Impl(QObject* audioParent)
        : sample(new QSoundEffect(audioParent))
    {
    }

    void setVolume(float volume)
    {
        sample->setVolume(volume);
    }

    void play(const QUrl& soundUrl)
    {
        sample->setSource(soundUrl);        
        sample->play();
    }

private:
    QSoundEffect* sample;
};

#elif defined(USE_MEDIAPLAYER)

#include <QMediaPlayer>

class AudioPlayer::Impl
{
public:
    Impl(QObject* audioParent)
        : player(new QMediaPlayer(audioParent, QMediaPlayer::StreamPlayback))
    {
    }

    void setVolume(float volume)
    {
        player->setVolume(volume * 100);
    }

    void play(const QUrl& soundUrl)
    {
        player->setMedia(soundUrl);
        player->play();
    }

private:
    QMediaPlayer* player;
};

#endif


AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , d(new Impl(this))
{
}

AudioPlayer::~AudioPlayer()
{
    delete d;
}

void AudioPlayer::setVolume(float volume)
{
    d->setVolume(volume);
}

void AudioPlayer::play(const QUrl &soundUrl)
{
    d->play(soundUrl);
}
