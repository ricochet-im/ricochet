#include "signals.hpp"
#include "context.hpp"
#include "error.hpp"
#include "ed25519.hpp"
#include "user.hpp"
#include "file_hash.hpp"

namespace tego
{
    type_erased_callback::type_erased_callback(type_erased_callback&& that)
    {
        *this = std::move(that);
    }

    type_erased_callback& type_erased_callback::operator=(type_erased_callback&& that)
    {
        this->data_ = std::move(that.data_);
        this->callback_ = that.callback_;
        that.callback_ = nullptr;

        return *this;
    }


    void type_erased_callback::invoke()
    {
        this->callback_(this->data_.get());
    }

    //
    // Callback Registery
    //

    callback_registry::callback_registry(tego_context* context)
    : context_(context)
    {
        TEGO_THROW_IF_NULL(context);
    }

    void callback_registry::push_back(type_erased_callback&& callback)
    {
        this->context_->callback_queue_.push_back(std::move(callback));
    }

    //
    // Callback Queue
    //

    callback_queue::callback_queue(tego_context* context)
    : context_(context)
    , terminating_(false)
    , mutex_()
    , pending_callbacks_()
    , worker_([](tego_context* ctx) -> void
    {
        auto& self = ctx->callback_queue_;

        // we use double buffering to ensure we can enqueue tasks
        // without blocking while we are executing previously
        // enqueued tasks
        decltype(self.pending_callbacks_) local_queue;

        // we keep spinning until termination is signaled
        while(!self.terminating_)
        {
            // acquire the queue's lock and swap our queues
            // the backend can now keep emitting callbacks
            // while we work through our queue of old ones
            {
                std::lock_guard<std::mutex> lock(self.mutex_);
                std::swap(local_queue, self.pending_callbacks_);
            }

            for(auto& callback : local_queue) {
                // acquire our context's lock so that we don't have two
                // threads potentially modifying internals
                std::lock_guard<std::mutex> lock(ctx->mutex_);
                try
                {
                    callback.invoke();
                }
                // swallow any throw exceptions
                catch(...) {};
            }
            // empty our our local queue
            local_queue.clear();

            // sleep before looking for more work
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

    }, context)
    {
        // empty constructor
    }

    callback_queue::~callback_queue()
    {
        // signal our worker thread to finish up and terminate
        terminating_ = true;
        worker_.join();
    }

    void callback_queue::push_back(type_erased_callback&& callback)
    {
        // acquire our lock and push into our vec
        if (!terminating_)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_callbacks_.push_back(std::move(callback));
        }
    }
}

//
// Implementation of our exposed callback setters
//

extern "C"
{
    #define TEGO_DEFINE_CALLBACK_SETTER(EVENT)\
    void tego_context_set_##EVENT##_callback(\
        tego_context_t* context,\
        tego_##EVENT##_callback_t callback,\
        tego_error_t** error)\
    {\
        return tego::translateExceptions([=]() -> void\
        {\
            TEGO_THROW_IF_NULL(context);\
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());\
            context->callback_registry_.register_##EVENT(callback);\
        }, error);\
    }

    TEGO_DEFINE_CALLBACK_SETTER(tor_error_occurred)
    TEGO_DEFINE_CALLBACK_SETTER(update_tor_daemon_config_succeeded)
    TEGO_DEFINE_CALLBACK_SETTER(tor_control_status_changed)
    TEGO_DEFINE_CALLBACK_SETTER(tor_process_status_changed)
    TEGO_DEFINE_CALLBACK_SETTER(tor_network_status_changed)
    TEGO_DEFINE_CALLBACK_SETTER(tor_bootstrap_status_changed)
    TEGO_DEFINE_CALLBACK_SETTER(tor_log_received)
    TEGO_DEFINE_CALLBACK_SETTER(host_user_state_changed)
    TEGO_DEFINE_CALLBACK_SETTER(chat_request_received)
    TEGO_DEFINE_CALLBACK_SETTER(chat_request_response_received)
    TEGO_DEFINE_CALLBACK_SETTER(message_received)
    TEGO_DEFINE_CALLBACK_SETTER(message_acknowledged)
    TEGO_DEFINE_CALLBACK_SETTER(file_transfer_request_received)
    TEGO_DEFINE_CALLBACK_SETTER(file_transfer_request_acknowledged)
    TEGO_DEFINE_CALLBACK_SETTER(file_transfer_request_response_received)
    TEGO_DEFINE_CALLBACK_SETTER(file_transfer_progress)
    TEGO_DEFINE_CALLBACK_SETTER(file_transfer_complete)
    TEGO_DEFINE_CALLBACK_SETTER(user_status_changed)
    TEGO_DEFINE_CALLBACK_SETTER(new_identity_created)
}
