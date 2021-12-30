#include "OutgoingContactRequest.h"

namespace shims
{
    OutgoingContactRequest::Status OutgoingContactRequest::getStatus() const
    {
        logger::trace();
        return this->status;
    }

    void OutgoingContactRequest::setStatus(Status newStatus)
    {
        if (this->status != newStatus)
        {
            emit this->statusChanged(this->status, newStatus);
            this->status = newStatus;
        }
    }

    void OutgoingContactRequest::setAccepted()
    {
        this->setStatus(Acknowledged);
        this->setStatus(Accepted);
    }

    void OutgoingContactRequest::setRejected()
    {
        this->setStatus(Acknowledged);
        this->setStatus(Rejected);

        emit this->rejected();
    }
}
