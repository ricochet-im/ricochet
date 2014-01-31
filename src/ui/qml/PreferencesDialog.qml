import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: preferencesWindow
    width: 600
    height: 500
    title: qsTr("Torsion Preferences")

    Button {
        anchors.centerIn: parent
        text: "Network Setup"
        onClicked: {
            var object = createDialog("NetworkSetupWizard.qml")
            object.visible = true
        }
    }
}

