import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

ApplicationWindow {
    id: preferencesWindow
    width: 600
    height: 500
    title: qsTr("Torsion Preferences")

    TabView {
        anchors.fill: parent
        anchors.margins: 8

        Tab {
            title: "Contacts"
            source: Qt.resolvedUrl("ContactPreferences.qml")
        }

        Tab {
            title: "Tor"
            source: Qt.resolvedUrl("TorPreferences.qml")
        }
    }
}
