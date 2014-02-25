import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Label {
    font.pointSize: 11
    text: {
        if (torControl.status === TorControl.Error)
            return "Connection failed"
        if (torControl.status < TorControl.Connected)
            return "Connecting..."

        if (torControl.torStatus === TorControl.TorUnknown ||
            torControl.torStatus === TorControl.TorOffline)
        {
            var bootstrap = torControl.bootstrapStatus
            if (bootstrap['recommendation'] === 'warn')
                return "Connection failed"
            else if (bootstrap['progress'] === undefined)
                return "Connecting..."
            else
                return "Connecting... (" + bootstrap['progress'] + "%)"
        }

        if (torControl.torStatus === TorControl.TorReady) {
            // Indicates whether we've verified that the hidden services is connectable
            if (userIdentity.isOnline)
                return "Online"
            else
                return "Connected"
        }
    }
}
