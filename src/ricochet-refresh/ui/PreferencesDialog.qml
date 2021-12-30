import QtQuick 2.2
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

ApplicationWindow {
    id: preferencesWindow
    width: 820
    minimumWidth: 820
    height: 400
    minimumHeight: 400
    title: qsTr("Ricochet Preferences")

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

        Accessible.role: Accessible.MenuBar
        //: Name of the tab bar for accessibility tech like screen readers
        Accessible.name: qsTr("Menu Tabs")

        /* QT will automatically set Accessible.text, also tabs fail to load if
         * you set any accessibility properties */
        Tab {
            //: Title of the general settings tab
            title: qsTr("General")
            source: Qt.resolvedUrl("GeneralPreferences.qml")
        }

        Tab {
            //: Title of the contacts list tab
            title: qsTr("Contacts")
            source: Qt.resolvedUrl("ContactPreferences.qml")
        }

        Tab {
            //: Title of the tor tab, contains tor settings and logs
            title: qsTr("Tor")
            source: Qt.resolvedUrl("TorPreferences.qml")
        }

        Tab {
            //: Title of the about tab, contains license information and ricochet version
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
