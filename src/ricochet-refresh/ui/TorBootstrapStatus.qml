import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: page
    spacing: 8

    property var bootstrap: torControl.bootstrapStatus
    onBootstrapChanged: {
        if (bootstrap['tag'] === "done")
            window.networkReady()
    }

    Label {
        //: Text status displayed when tor daemon is connecting, \u2026 is ellipsis
        text: qsTr("Connecting to the Tor network\u2026")
        font.bold: true

        Accessible.role: Accessible.StaticText
        Accessible.name: text
        //: Description of the connection status text, used by accessibility tech like screen readers
        Accessible.description: qsTr("Connection status");
    }

    ProgressBar {
        width: parent.width
        maximumValue: 100
        indeterminate: bootstrap.progress === undefined
        value: bootstrap.progress === undefined ? 0 : bootstrap.progress

        Accessible.role: Accessible.ProgressBar
        //: Description of the bootstrap progress bar, used by accessibility tech like screen readers
        Accessible.description: qsTr("Connection progress");
    }

    Label {
        text: (bootstrap['warning'] !== undefined ) ? bootstrap['warning'] : bootstrap['summary']
        textFormat: Text.PlainText
    }

    TorLogDisplay {
        id: logDisplay
        width: parent.width
        height: 0
        visible: height > 0

        Behavior on height {
            SmoothedAnimation {
                easing.type: Easing.InOutQuad
                velocity: 1500
            }
        }
    }

    RowLayout {
        width: parent.width

        Button {
            //: Label for button which moves us back to previous screen
            text: qsTr("Back")
            onClicked: window.back()

            //: Label for button which moves us back to previous screen
            Accessible.name: qsTr("Back")
            Accessible.onPressAction: window.back()
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            text: logDisplay.height ?
                  //: Label for button which hides the tor logs
                  qsTr("Hide Details") :
                  //: Label for button which shows the tor logs
                  qsTr("Show Details")
            onClicked: {
                if (logDisplay.height)
                    logDisplay.height = 0
                else
                    logDisplay.height = 300
            }

            Accessible.name: text
            Accessible.role: Accessible.Button
            Accessible.description: logDisplay.height ?
                                    //: Description of hide details button, used by accessibility tech like screen readers
                                    qsTr("Hides the tor progress log") :
                                    //: Description of show details button, used by accessibility tech like screen readers
                                    qsTr("Shows the tor progress log")
            Accessible.onPressAction: {
                if (logDisplay.height) logDisplay.height = 0
                else logDisplay.height = 300
            }
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            //: Label for button which closes the tor connection window
            text: qsTr("Done")
            isDefault: true
            enabled: bootstrap.tag === "done"
            onClicked: window.visible = false

            Accessible.name: text
            Accessible.role: Accessible.Button
            Accessible.onPressAction: window.visible = false
        }
    }
}

