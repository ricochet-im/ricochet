import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: page
    spacing: 8

    Label {
        width: parent.width
        text: "Does this computer need a proxy to access the internet?"
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: page.width

        GridLayout {
            anchors.fill: parent
            columns: 2

            Label {
                text: "Proxy type:"
                color: proxyPalette.text
            }
            ComboBox {
                id: proxyType
                model: ListModel {
                    ListElement { text: "None"; type: "" }
                    ListElement { text: "SOCKS 4"; type: "socks4" }
                    ListElement { text: "SOCKS 5"; type: "socks5" }
                    ListElement { text: "HTTP / HTTPS"; type: "http" }
                }
                textRole: "text"
                property string selectedType: currentIndex >= 0 ? model.get(currentIndex).type : ""
                SystemPalette {
                    id: proxyPalette
                    colorGroup: proxyType.selectedType == "" ? SystemPalette.Disabled : SystemPalette.Active
                }
            }

            Label {
                text: "Address:"
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    Layout.fillWidth: true
                    enabled: proxyType.selectedType
                    placeholderText: "IP address or hostname"
                }
                Label {
                    text: "Port:"
                    color: proxyPalette.text
                }
                TextField {
                    Layout.preferredWidth: 50
                    enabled: proxyType.selectedType
                }
            }

            Label {
                text: "Username:"
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true

                TextField {
                    Layout.fillWidth: true
                    enabled: proxyType.selectedType
                    placeholderText: "Optional"
                }
                Label {
                    text: "Password:"
                    color: proxyPalette.text
                }
                TextField {
                    Layout.fillWidth: true
                    enabled: proxyType.selectedType
                    placeholderText: "Optional"
                }
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: "Does this computer's Internet connection go through a firewall " +
              "that only allows connections to certain ports?"
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        // Workaround OS X visual bug
        height: Math.max(implicitHeight, 40)
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Allowed ports:"
            }
            TextField {
                id: allowedPorts
                Layout.fillWidth: true
            }
            Label {
                text: "Example: 80,443"
                SystemPalette { id: disabledPalette; colorGroup: SystemPalette.Disabled }
                color: disabledPalette.text
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: "If this computer's Internet connection is censored, you will need " +
              "to obtain and use bridge relays."
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        ColumnLayout {
            anchors.fill: parent
            Label {
                text: "Enter one or more bridge relays (one per line):"
            }
            TextArea {
                Layout.fillWidth: true
                Layout.preferredHeight: allowedPorts.height * 2
                tabChangesFocus: true
            }
        }
    }

    RowLayout {
        width: parent.width

        Button {
            text: "Back"
            onClicked: window.back()
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            text: "Connect"
            isDefault: true
            onClicked: window.openBootstrap()
        }
    }
}
