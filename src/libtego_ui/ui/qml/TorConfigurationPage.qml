import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Column {
    id: setup
    spacing: 8

    property alias proxyType: proxyTypeField.selectedType
    property alias proxyAddress: proxyAddressField.text
    property alias proxyPort: proxyPortField.text
    property alias proxyUsername: proxyUsernameField.text
    property alias proxyPassword: proxyPasswordField.text
    property alias allowedPorts: allowedPortsField.text
    property alias bridges: bridgesField.text

    function reset() {
        proxyTypeField.currentIndex = 0
        proxyAddress = ''
        proxyPort = ''
        proxyUsername = ''
        proxyPassword = ''
        allowedPorts = ''
        bridges = ''
    }

    function save() {
        var conf = {};
        conf.disableNetwork = "0";
        conf.proxyType = proxyType;
        conf.proxyAddress = proxyAddress;
        conf.proxyPort = proxyPort;
        conf.proxyUsername = proxyUsername;
        conf.proxyPassword = proxyPassword;
        conf.allowedPorts = allowedPorts.trim().length > 0 ? allowedPorts.trim().split(',') : [];
        conf.bridges = bridges.trim().length > 0 ? bridges.trim().split('\n') : [];

        var command = torControl.setConfiguration(conf)
        command.finished.connect(function() {
            if (command.successful) {
                if (torControl.hasOwnership)
                    torControl.saveConfiguration()
                window.openBootstrap()
            } else
                console.log("SETCONF error:", command.errorMessage)
        })
    }

    Label {
        width: parent.width
        text: qsTr("Does this computer need a proxy to access the internet?")
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: setup.width

        GridLayout {
            anchors.fill: parent
            columns: 2

            Label {
                text: qsTr("Proxy type:")
                color: proxyPalette.text
            }
            ComboBox {
                id: proxyTypeField
                property string none: qsTr("None")
                model: [
                    { "text": qsTr("None"), "type": "" },
                    { "text": "SOCKS 4", "type": "socks4" },
                    { "text": "SOCKS 5", "type": "socks5" },
                    { "text": "HTTPS", "type": "https" },
                ]
                textRole: "text"
                property string selectedType: currentIndex >= 0 ? model[currentIndex].type : ""

                SystemPalette {
                    id: proxyPalette
                    colorGroup: setup.proxyType == "" ? SystemPalette.Disabled : SystemPalette.Active
                }
            }

            Label {
                text: qsTr("Address:")
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true
                TextField {
                    id: proxyAddressField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: qsTr("IP address or hostname")
                }
                Label {
                    text: qsTr("Port:")
                    color: proxyPalette.text
                }
                TextField {
                    id: proxyPortField
                    Layout.preferredWidth: 50
                    enabled: setup.proxyType
                }
            }

            Label {
                text: qsTr("Username:")
                color: proxyPalette.text
            }
            RowLayout {
                Layout.fillWidth: true

                TextField {
                    id: proxyUsernameField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: qsTr("Optional")
                }
                Label {
                    text: qsTr("Password:")
                    color: proxyPalette.text
                }
                TextField {
                    id: proxyPasswordField
                    Layout.fillWidth: true
                    enabled: setup.proxyType
                    placeholderText: qsTr("Optional")
                }
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: qsTr("Does this computer's Internet connection go through a firewall that only allows connections to certain ports?")
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        // Workaround OS X visual bug
        height: Math.max(implicitHeight, 40)
        RowLayout {
            anchors.fill: parent
            Label {
                text: qsTr("Allowed ports:")
            }
            TextField {
                id: allowedPortsField
                Layout.fillWidth: true
            }
            Label {
                text: qsTr("Example: 80,443")
                SystemPalette { id: disabledPalette; colorGroup: SystemPalette.Disabled }
                color: disabledPalette.text
            }
        }
    }

    Item { height: 4; width: 1 }

    Label {
        width: parent.width
        text: qsTr("If this computer's Internet connection is censored, you will need to obtain and use bridge relays.")
        wrapMode: Text.Wrap
    }

    GroupBox {
        width: parent.width
        ColumnLayout {
            anchors.fill: parent
            Label {
                text: qsTr("Enter one or more bridge relays (one per line):")
            }
            TextArea {
                id: bridgesField
                Layout.fillWidth: true
                Layout.preferredHeight: allowedPortsField.height * 2
                tabChangesFocus: true
            }
        }
    }

    RowLayout {
        width: parent.width

        Button {
            text: qsTr("Back")
            onClicked: window.back()
        }

        Item { height: 1; Layout.fillWidth: true }

        Button {
            text: qsTr("Connect")
            isDefault: true
            onClicked: {
                setup.save()
            }
        }
    }
}
