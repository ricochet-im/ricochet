import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    CheckBox {
        //: Text description of an option to use one single program window for the contact list and the chats
        text: qsTr("Use a single window for conversations")
        checked: uiSettings.data.combinedChatWindow || false
        onCheckedChanged: {
            uiSettings.write("combinedChatWindow", checked)
        }

        Accessible.role: Accessible.CheckBox
        Accessible.name: text
        Accessible.onPressAction: {
            uiSettings.write("combinedChatWindow", checked)
        }
    }

    CheckBox {
        //: Text description of an option to play audio notifications when contacts log in, log out, and send messages
        text: qsTr("Play audio notifications")
        checked: uiSettings.data.playAudioNotification || false
        onCheckedChanged: {
            uiSettings.write("playAudioNotification", checked)
        }

        Accessible.role: Accessible.CheckBox
        Accessible.name: text
        Accessible.onPressAction: {
            uiSettings.write("playAudioNotification", checked)
        }
    }
    RowLayout {
        Item { width: 16 }

        Label {
            //: Label for a slider used to adjust audio notification volume
            text: qsTr("Volume")
            Accessible.role: Accessible.StaticText
            Accessible.name: text
        }

        Slider {
            maximumValue: 1.0
            updateValueWhileDragging: false
            enabled: uiSettings.data.playAudioNotification || false
            value: uiSettings.read("notificationVolume", 0.75)
            onValueChanged: {
                uiSettings.write("notificationVolume", value)
            }

            Accessible.role: Accessible.Slider
            //: Name of the slider used to adjust audio notification volume for accessibility tech like screen readers
            Accessible.name: qsTr("Volume")
            Accessible.onIncreaseAction: {
                value += 0.125 // 8 volume settings
            }
            Accessible.onDecreaseAction: {
                value -= 0.125
            }
        }
    }

    RowLayout {
        z: 2
        Label {
            //: Label for combobox where users can specify the UI language
            text: qsTr("Language")
            Accessible.role: Accessible.StaticText
            Accessible.name: text
        }

        ComboBox {
            id: languageBox
            model: languageModel
            textRole: "nativeName"
            currentIndex: languageModel.rowForLocaleID(uiSettings.data.language)
            Layout.minimumWidth: 200

            LanguagesModel {
                id: languageModel
            }

            onActivated: {
                var localeID = languageModel.localeID(index)
                uiSettings.write("language", localeID)
                restartBubble.displayed = true
                bubbleResetTimer.start()
            }

            Bubble {
                id: restartBubble
                target: languageBox
                text: qsTr("Restart Ricochet to apply changes")
                displayed: false
                horizontalAlignment: Qt.AlignRight

                Timer {
                    id: bubbleResetTimer
                    interval: 3000
                    onTriggered: restartBubble.displayed = false
                }
            }
            Accessible.role: Accessible.ComboBox
            //: Name of the combobox used to select UI langauge for accessibility tech like screen readers
            Accessible.name: qsTr("Language")
            //: Description of what the language combox is for for accessibility tech like screen readers
            Accessible.description: qsTr("What language ricochet will use")
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
