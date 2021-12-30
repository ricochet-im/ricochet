#pragma once

namespace shims
{
    class ContactUser;
    class OutgoingContactRequest : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(OutgoingContactRequest)
        Q_ENUMS(Status)

        Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
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

        Status getStatus() const;

        void setStatus(Status status);

        void setAccepted();
        void setRejected();


    signals:
        void statusChanged(int newStatus, int oldStatus);
        void rejected();

    private:
        Status status = shims::OutgoingContactRequest::Pending;
    };
}