import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import org.torsionim.torsion 1.0

Label {
    font.pointSize: 12
    font.bold: true
    text: {
        if (torManager.status === TorControlManager.Error)
            return "Configuration error"
        if (torManager.status < TorControlManager.Connected)
            return "Starting up..."
        if (torManager.torStatus === TorControlManager.TorUnknown ||
            torManager.torStatus === TorControlManager.TorOffline)
            return "Offline"
        if (torManager.torStatus === TorControlManager.TorBootstrapping)
            return "Connecting..."
        if (torManager.torStatus === TorControlManager.TorReady) {
            if (userIdentity.isOnline)
                return "Online"
            else if (userIdentity.isPublished)
                return "Verifying..."
            else
                return "Publishing..."
        }
    }
}
