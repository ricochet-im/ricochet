import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ApplicationWindow {
    id: preferencesWindow
    width: 550
    minimumWidth: 550
    height: 400
    minimumHeight: 400
    title: qsTr("Ricochet Preferences")
    flags: Qt.Dialog

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
            title: qsTr("General")
            source: Qt.resolvedUrl("GeneralPreferences.qml")
        }

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

    Action {
        shortcut: StandardKey.Close
        onTriggered: preferencesWindow.close()
    }

    Action {
        shortcut: "Escape"
        onTriggered: preferencesWindow.close()
    }
}
