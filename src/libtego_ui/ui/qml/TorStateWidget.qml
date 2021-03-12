import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import im.ricochet 1.0

Label {
    text: {
        if (torControl.status === TorControl.Error)
            return qsTr("Connection failed")
        if (torControl.status < TorControl.Connected) {
            //: \u2026 is ellipsis
            return qsTr("Connecting\u2026")
        }

        if (torControl.torStatus === TorControl.TorUnknown ||
            torControl.torStatus === TorControl.TorOffline)
        {
            var bootstrap = torControl.bootstrapStatus
            if (bootstrap['recommendation'] === 'warn')
                return qsTr("Connection failed")
            else if (bootstrap['progress'] === undefined)
                return qsTr("Connecting\u2026")
            else {
                //: %1 is progress percentage, e.g. 100
                return qsTr("Connecting\u2026 (%1%)").arg(bootstrap['progress'])
            }
        }

        if (torControl.torStatus === TorControl.TorReady) {
            // Indicates whether we've verified that the hidden services is connectable
            if (userIdentity.isOnline)
                return qsTr("Online")
            else
                return qsTr("Connected")
        }
    }

    Accessible.name: qsTr("Tor status") // todo: translation
    Accessible.role: Accessible.StaticText
    Accessible.description: text
}
