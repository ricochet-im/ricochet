#pragma once

namespace shims
{
    class IncomingRequestManager : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(IncomingRequestManager)

        Q_PROPERTY(QList<QObject*> requests READ requestObjects NOTIFY requestsChanged)
    public:
        QList<QObject*> requestObjects() const;
    signals:
        void requestsChanged();
    };
}