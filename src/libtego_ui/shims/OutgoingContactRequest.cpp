#include "OutgoingContactRequest.h"

namespace shims
{
    OutgoingContactRequest::Status OutgoingContactRequest::getStatus() const
    {
        logger::trace();
        return this->status;
    }

    void OutgoingContactRequest::setStatus(Status status)
    {
        if (this->status != status)
        {
            emit this->statusChanged(this->status, status);
            this->status = status;
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