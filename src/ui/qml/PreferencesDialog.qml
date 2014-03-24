import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

ApplicationWindow {
    id: preferencesWindow
    width: 600
    height: 500
    title: qsTr("Torsion Preferences")

    signal closed
    onVisibleChanged: if (!visible) closed()

    property string initialPage
    property var initialPageProperties: { }

    Component.onCompleted: {
        if (initialPage != "") {
            initialPage = Qt.resolvedUrl(initialPage)
            for (var i = 0; i < tabs.count; i++) {
                if (tabs.getTab(i).source == initialPage) {
                    tabs.currentIndex = i
                    var item = tabs.getTab(i).item
                    for (var key in initialPageProperties) {
                        item[key] = initialPageProperties[key]
                    }
                }
            }
        }
    }

    TabView {
        id: tabs
        anchors.fill: parent
        anchors.margins: 8

        Tab {
            title: qsTr("Contacts")
            source: Qt.resolvedUrl("ContactPreferences.qml")
        }

        Tab {
            title: qsTr("Tor")
            source: Qt.resolvedUrl("TorPreferences.qml")
        }

        Tab {
            title: qsTr("About")
            source: Qt.resolvedUrl("AboutPreferences.qml")
        }
    }
}
