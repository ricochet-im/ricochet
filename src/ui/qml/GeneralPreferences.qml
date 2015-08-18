import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    CheckBox {
        text: qsTr("Use a single window for conversations")
        checked: uiSettings.data.combinedChatWindow || false
        onCheckedChanged: {
            uiSettings.write("combinedChatWindow", checked)
        }
    }

    CheckBox {
        text: qsTr("Open links in default browser without prompting")
        checked: uiSettings.data.alwaysOpenBrowser || false
        onCheckedChanged: {
            uiSettings.write("alwaysOpenBrowser", checked)
        }
    }

    CheckBox {
        text: qsTr("Hide tray icon")
        checked: uiSettings.data.hideTrayIcon || false
        onCheckedChanged: {
            uiSettings.write("hideTrayIcon", checked)
        }
    }

    CheckBox {
        text: qsTr("Show desktop notifications when new message incomes")
        checked: uiSettings.data.showNotifications || false
        onCheckedChanged: {
            uiSettings.write("showNotifications", checked)
        }
    }

    CheckBox {
        text: qsTr("'Movie-style' notification (big, centered on the screen)")
        checked: uiSettings.data.movieStyleNotification || false
        enabled: uiSettings.data.showNotifications || false
        onCheckedChanged: {
            uiSettings.write("movieStyleNotification", checked)
        }
    }

    RowLayout {
        Item { width: 16 }

        Label { text: qsTr("Duration") }

        Slider {
            id: notificationDurationValue
            stepSize: 1.0
            maximumValue: 10.0
            updateValueWhileDragging: false
            enabled: uiSettings.data.showNotifications || false
            value: uiSettings.read("notificationDuration", 4.0)
            onValueChanged: uiSettings.write("notificationDuration", value)

            // must be set after loading value from settings, in other case value is set to minimum
            Component.onCompleted: { minimumValue = 1.0 }
        }

        Label {
            text: qsTr("%1 sec").arg(notificationDurationValue.value)
        }
    }

    CheckBox {
        text: qsTr("Play audio notifications")
        checked: uiSettings.data.playAudioNotification || false
        onCheckedChanged: {
            uiSettings.write("playAudioNotification", checked)
        }
    }

    RowLayout {
        Item { width: 16 }

        Label { text: qsTr("Volume") }

        Slider {
            maximumValue: 1.0
            updateValueWhileDragging: false
            enabled: uiSettings.data.playAudioNotification || false
            value: uiSettings.read("notificationVolume", 0.75)
            onValueChanged: {
                uiSettings.write("notificationVolume", value)
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
