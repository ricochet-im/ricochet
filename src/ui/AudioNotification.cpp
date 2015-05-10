/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
