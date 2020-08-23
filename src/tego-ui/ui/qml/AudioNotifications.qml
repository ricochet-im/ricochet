import QtQuick 2.0
import QtMultimedia 5.0

QtObject {
    id: audioNotifications

    property real volume: uiSettings.data.notificationVolume

    property SoundEffect message: SoundEffect {
        source: "qrc:/sounds/message.wav"
        volume: audioNotifications.volume
    }

    property SoundEffect contactOnline: SoundEffect {
        source: "qrc:/sounds/online.wav"
        volume: audioNotifications.volume
    }
}
