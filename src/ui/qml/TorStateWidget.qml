import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Label {
    font.pointSize: 12
    font.bold: true
    text: {
        if (torManager.status === TorControl.Error)
            return "Configuration error"
        if (torManager.status < TorControl.Connected)
            return "Starting up..."
        if (torManager.torStatus === TorControl.TorUnknown ||
            torManager.torStatus === TorControl.TorOffline)
            return "Offline"
        if (torManager.torStatus === TorControl.TorBootstrapping)
            return "Connecting..."
        if (torManager.torStatus === TorControl.TorReady) {
            if (userIdentity.isOnline)
                return "Online"
            else if (userIdentity.isPublished)
                return "Verifying..."
            else
                return "Publishing..."
        }
    }
}
