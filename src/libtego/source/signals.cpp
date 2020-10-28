#include "signals.hpp"
#include "context.hpp"

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
        this->delete_ = that.delete_;
        that.delete_ = nullptr;

        return *this;
    }

    void type_erased_callback::invoke()
    {
        this->callback_(this->data_.get());
    }

    callback_registry::callback_registry(tego_context* context)
    : context_(context)
    {

    }

    //
    // Callback Queue
    //

    callback_queue::callback_queue(tego_context* context)
    : context_(context)
    , terminating_(false)
    , mutex_({})
    , worker_([](tego_context* ctx) -> void
    {
        logger::trace();

        auto& self = ctx->callback_queue_;

        // we use double buffering to ensure we can enqueue callbacks
        // while we are executing previously enqueued ones
        decltype(self.pending_callbacks_) local_queue;

        // we keep spinning until

        while(!self.terminating_)
        {
            // acquire the queue's lock and swap our queues
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
    , pending_callbacks_()
    {

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

