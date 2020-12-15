#include "OutgoingContactRequest.h"

#include "core/OutgoingContactRequest.h"

namespace shims
{
    OutgoingContactRequest::Status OutgoingContactRequest::status() const
    {
        logger::trace();
        return static_cast<OutgoingContactRequest::Status>(outgoingContactRequest->status());
    }

    QString OutgoingContactRequest::rejectMessage() const
    {
        logger::trace();
        return outgoingContactRequest->rejectMessage();
    }

    void OutgoingContactRequest::setOutgoingContactRequest(::OutgoingContactRequest* outgoingContactRequest)
    {
        // remove existing connections
        if (this->outgoingContactRequest)
        {
            disconnect(this->outgoingContactRequest, 0, 0, 0);
        }

        this->outgoingContactRequest = outgoingContactRequest;

        logger::trace();
        // wire up slots to forward
        connect(
            outgoingContactRequest,
            &::OutgoingContactRequest::statusChanged,
            [self=this](int newStatus, int oldStatus)
            {
                logger::trace();
                emit self->statusChanged(newStatus, oldStatus);
            });

        connect(
            outgoingContactRequest,
            &::OutgoingContactRequest::rejected,
            [self=this]()
            {
                logger::trace();
                emit self->rejected();
            });
    }
}