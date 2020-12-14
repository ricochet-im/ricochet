#pragma once

class OutgoingContactRequest;

namespace shims
{
    class ContactUser;
    class OutgoingContactRequest : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(OutgoingContactRequest)
        Q_ENUMS(Status)

        Q_PROPERTY(Status status READ status NOTIFY statusChanged)
        Q_PROPERTY(QString rejectMessage READ rejectMessage NOTIFY rejected)

    public:
        enum Status
        {
            Pending,
            Acknowledged,
            Accepted,
            Error,
            Rejected,
            FirstResult = Accepted
        };

        OutgoingContactRequest() = default;

        Status status() const;
        QString rejectMessage() const;

    signals:
        void statusChanged(int newStatus, int oldStatus);
        void rejected(const QString &reason);

    private:
        void setOutgoingContactRequest(::OutgoingContactRequest*);
        ::OutgoingContactRequest* outgoingContactRequest = nullptr;

        friend class shims::ContactUser;
    };
}