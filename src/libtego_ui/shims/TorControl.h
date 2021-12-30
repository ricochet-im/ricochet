#pragma once

#include "TorCommand.h"

namespace shims
{
	// shim version of Tor::ToControl with just the functionality requried by the UI
    class TorControl : public QObject
    {
        Q_OBJECT
        Q_ENUMS(Status TorStatus)

        Q_PROPERTY(bool hasOwnership READ hasOwnership CONSTANT)
        Q_PROPERTY(QString torVersion READ torVersion CONSTANT)
        // Status of the control connection
        Q_PROPERTY(Status status READ status NOTIFY statusChanged)
        // Status of Tor (and whether it believes it can connect)
        Q_PROPERTY(TorStatus torStatus READ torStatus NOTIFY torStatusChanged)
        Q_PROPERTY(QVariantMap bootstrapStatus READ bootstrapStatus NOTIFY bootstrapStatusChanged)
        // uses statusChanged like actual backend implementation
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY statusChanged)
    public:
        enum Status
        {
            Error = -1,
            NotConnected,
            Connecting,
            Authenticating,
            Connected
        };

        enum TorStatus
        {
            TorError = -1,
            TorUnknown,
            TorOffline,
            TorReady
        };

        Q_INVOKABLE QObject *setConfiguration(const QVariantMap &options);
        Q_INVOKABLE void saveConfiguration();

        TorControl(tego_context_t* context);

        /* Ownership means that tor is managed by this socket, and we
         * can shut it down, own its configuration, etc. */
        bool hasOwnership() const;

        QString torVersion() const;
        Status status() const;
        TorStatus torStatus() const;
        QVariantMap bootstrapStatus() const;
        QString errorMessage() const;

        void setStatus(Status);
        void setTorStatus(TorStatus);
        void setErrorMessage(const QString&);

        static TorControl* torControl;
        TorControlCommand* m_setConfigurationCommand = nullptr;
        Status m_status = NotConnected;
        TorStatus m_torStatus = TorUnknown;
        QString m_errorMessage;

    signals:
        void statusChanged(int newStatus, int oldStatus);
        void torStatusChanged(int newStatus, int oldStatus);
        void bootstrapStatusChanged();

    private:
        tego_context_t* context;
    };
}