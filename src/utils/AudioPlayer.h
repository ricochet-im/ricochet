#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>

class AudioPlayer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AudioPlayer)

public:
    AudioPlayer(QObject *parent = 0);
    ~AudioPlayer();

public slots:
    void setVolume(float volume);
    void play(const QUrl& soundUrl);

private:
    class Impl;
    Impl* const d;
};

#endif // AUDIOPLAYER_H
