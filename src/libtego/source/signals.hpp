#pragma once

namespace tego
{
    // allows us to marshall lambda functions and their data to
    // call them later
    class type_erased_callback
    {
    public:
        type_erased_callback() = default;
        type_erased_callback(type_erased_callback&&);
        type_erased_callback& operator=(type_erased_callback&&);

        template<typename FUNC>
        type_erased_callback(FUNC&& func)
        {
            // heap allocated a moved copy of the passed in function
            data_ = {
                new FUNC(std::move(func)),
                // deleter function casts raw ptr back to FUNC and deletes
                [](void* data) {
                    auto* lambda = static_cast<FUNC*>(data);
                    delete lambda;
                }
            };

            // save of a function for invoking func
            callback_ = [](void* data) {
                auto* lambda = static_cast<FUNC*>(data);
                (*lambda)();
            };
        }
        void invoke();
    private:
        std::unique_ptr<void, void(*)(void*)> data_ = {nullptr, nullptr};
        void (*callback_)(void* data) = nullptr;
        void (*delete_)(void* data) = nullptr;
    };

    enum class signal_handle : size_t
    {
        tor_state_changed,
        tor_log_received,
        chat_request_received,
        message_received,
        user_status_changed,

        count
    };


    class callback_registry
    {
    public:
        callback_registry(tego_context* context);

        tego_tor_state_changed_callback_t tor_state_changed_ = nullptr;
        void register_tor_state_changed(tego_tor_state_changed_callback_t cb)
        {
            tor_state_changed_ = cb;
        }
        template<typename... ARGS>
        void emit_tor_state_changed(ARGS&&... args)
        {
            if (tor_state_changed_ != nullptr)
            {
                type_erased_callback tec = type_erased_callback{
                    [=,callback=tor_state_changed_]() -> void
                    {
                        callback(std::forward<ARGS>(args)...);
                    }
                };
                context_->callback_queue_->push_back(std::move(tec));
            }
        }
    private:
        tego_context* context_ = nullptr;
    };


    /*
     * callback_queue holds onto a queue of callbacks. Libtego internals
     * enqueue callbacks and the callback queue executes them on a
     * background worker thread
     */
    class callback_queue
    {
    public:
        callback_queue(tego_context* context);
        ~callback_queue();

        void push_back(type_erased_callback&&);
    private:
        tego_context* context_;

        std::atomic_bool terminating_;
        std::mutex mutex_;
        std::thread worker_;
        // this queue is protected by mutex_ within worker_ thread and callback_queue methods
        std::vector<type_erased_callback> pending_callbacks_;
    };
}