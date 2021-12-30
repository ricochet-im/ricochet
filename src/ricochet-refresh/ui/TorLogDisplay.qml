import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0

TextArea {
    id: logDisplay
    readOnly: true
    text: torInstance.logMessages.join('\n')
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap

    Connections {
        target: torInstance
        function onLogMessage(message) {
            logDisplay.append(message)
        }
    }

    //: Name of the text field containg the tor logs, used by accessibility tech such as screen readers
    Accessible.name: qsTr("Tor log")
    Accessible.description: text // XXX: seems like a bad idea to have the entire log read out
}
