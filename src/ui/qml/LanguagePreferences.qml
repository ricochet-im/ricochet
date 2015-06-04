import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    property string previousLanguage: uiSettings.data.language

    ExclusiveGroup {
        id: languageGroup
    }

    Item { height: 8 }

    Label {
        Layout.fillWidth: true
        text: qsTr("Select Language")
    }

    Item { height: 10 }

    GridLayout {
        columns: 2

        Repeater {
            model: LanguagesModel { }
            delegate: RadioButton {
                id: languageSelection
                Layout.fillWidth: true
                text: nativeName
                checked: localeID === uiSettings.data.language
                exclusiveGroup: languageGroup
                onCheckedChanged: {
                    if (checked && previousLanguage !== localeID) {
                        restartNotification.visible = true
                        uiSettings.write("language", localeID)
                    }
                }
            }
        }
    }

    Item { height: 15 }

    Label {
        id: restartNotification
        text: qsTr("Restart Ricochet to apply changes")
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        visible: false
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
