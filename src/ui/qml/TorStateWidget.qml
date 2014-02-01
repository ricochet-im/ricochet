import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Label {
    font.pointSize: 12
    font.bold: true
    text: {
        if (torControl.status === TorControl.Error)
            return "Configuration error"
        if (torControl.status < TorControl.Connected)
            return "Starting up..."
        if (torControl.torStatus === TorControl.TorUnknown ||
            torControl.torStatus === TorControl.TorOffline)
            return "Offline"
        if (torControl.torStatus === TorControl.TorBootstrapping)
            return "Connecting..."
        if (torControl.torStatus === TorControl.TorReady) {
            if (userIdentity.isOnline)
                return "Online"
            else if (userIdentity.isPublished)
                return "Verifying..."
            else
                return "Publishing..."
        }
    }
}
