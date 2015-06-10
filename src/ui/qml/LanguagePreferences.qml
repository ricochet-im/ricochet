import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
    }

    QtObject {
        id: local
        property string previousLanguage: uiSettings.data.language
    }

    ExclusiveGroup {
        id: languageGroup
    }

    Label {
        Layout.fillWidth: true
        text: qsTr("Select Language")
    }

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
                    if (checked && local.previousLanguage !== localeID) {
                        restartNotification.visible = true
                        uiSettings.write("language", localeID)
                    }
                }
            }
        }
    }

    Label {
        id: restartNotification
        text: qsTr("Restart Ricochet to apply changes!")
        visible: false
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
