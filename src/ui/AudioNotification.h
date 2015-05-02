#ifndef AUDIONOTIFICATION_H
#define AUDIONOTIFICATION_H

#include <QObject>
#include <QUrl>
class AudioPlayer;

class AudioNotification : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(float volume READ getVolume WRITE setVolume NOTIFY volumeChanged)

public:
    AudioNotification(QObject *parent = 0);
    ~AudioNotification();

signals:
    void enabledChanged();
    void volumeChanged();

public slots:
    void setEnabled(bool);
    void setVolume(float volume);

    bool isEnabled() const;
    float getVolume() const;

    void playIncomingMessage();
    void playContactOnline();

private:
    AudioPlayer* const audioPlayer;

    bool notificationsEnabled;
    float volume;

    QUrl incomingMessageSound;
    QUrl contactOnlineSound;
};

#endif // AUDIONOTIFICATION_H
