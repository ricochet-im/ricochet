#include "AudioNotification.h"
#include "utils/AudioPlayer.h"
#include <QFileInfo>
#include <QApplication>

AudioNotification::AudioNotification(QObject *parent)
    : QObject(parent)
    , audioPlayer(new AudioPlayer(this))
    , notificationsEnabled(true)
    , incomingMessageSound(QUrl(QStringLiteral("qrc:/sounds/message.wav")))
    , contactOnlineSound(QUrl(QStringLiteral("qrc:/sounds/online.wav")))
{
}

AudioNotification::~AudioNotification()
{
}

void AudioNotification::setEnabled(bool enabled)
{
    if ( enabled != notificationsEnabled )
    {
        notificationsEnabled = enabled;
        emit enabledChanged();
    }
}

bool AudioNotification::isEnabled() const
{
    return notificationsEnabled;
}

void AudioNotification::setVolume(float volume)
{
    if (this->volume != volume )
    {
        this->volume = volume;
        audioPlayer->setVolume(volume);
        emit volumeChanged();
    }
}

float AudioNotification::getVolume() const
{
    return volume;
}

void AudioNotification::playIncomingMessage()
{
    if (notificationsEnabled) audioPlayer->play(incomingMessageSound);
}

void AudioNotification::playContactOnline()
{
    if (notificationsEnabled) audioPlayer->play(contactOnlineSound);
}
