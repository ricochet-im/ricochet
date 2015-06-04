import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ColumnLayout {
    anchors {
        fill: parent
        margins: 8
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
            model: languagesModel
            delegate: RadioButton {
                Layout.fillWidth: true
                text: nativeName
                checked: localeID === uiSettings.data.language
                exclusiveGroup: languageGroup
                onCheckedChanged: {
                    if ( checked ) {
                        uiSettings.write("language", localeID)
                    }
                }
            }
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }
}
