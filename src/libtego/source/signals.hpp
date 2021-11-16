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
    };

    /*
     * The callback_register class keeps track of provided user callbacks
     * and lets us register them via register_X functions. Libtego internals
     * trigger callbacks by way of the emit_EVENT functions. The callback
     * registry is per-tego_context
     */

    class callback_registry
    {
    public:
        callback_registry(tego_context* context);

        /*
         * Each callback X has a register_X function, an emit_X function, and
         * a cleanup_X_args function
         *
         * It is assumed that a callback always sends over the tego_context_t* as
         * the first argument
         */
        #define TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(EVENT, ...)\
        private:\
            tego_##EVENT##_callback_t EVENT##_ = nullptr;\
        public:\
            void register_##EVENT(tego_##EVENT##_callback_t cb)\
            {\
                EVENT##_ = cb;\
            }\
            template<typename... ARGS>\
            void emit_##EVENT(ARGS&&... args)\
            {\
                if (EVENT##_ != nullptr) {\
                    push_back(\
                        [=, context=context_, callback=EVENT##_]() mutable -> void\
                        {\
                            callback(context, std::forward<ARGS>(args)...);\
                            cleanup_args(std::forward<ARGS>(args)...);\
                        }\
                    );\
                }\
            }\
        private:

        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_error_occurred, tego_tor_error_origin_t, tego_error_t*);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(update_tor_daemon_config_succeeded, tego_bool_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_control_status_changed, tego_tor_control_status_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_process_status_changed, tego_tor_process_status_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_network_status_changed, tego_tor_network_status_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_bootstrap_status_changed, int32_t, tego_tor_bootstrap_tag_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(tor_log_received, char*, size_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(host_user_state_changed, tego_host_user_state_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(chat_request_received, tego_user_id_t*, char*, size_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(chat_request_response_received, tego_user_id_t*, tego_bool_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(message_received, tego_user_id_t*, tego_time_t, tego_message_id_t, char*, size_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(message_acknowledged, tego_user_id_t*, tego_message_id_t, tego_bool_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(file_transfer_request_received, tego_user_id_t*, tego_file_transfer_id_t, char*, size_t, uint64_t, tego_file_hash_t*);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(file_transfer_request_acknowledged, tego_user_id_t*, tego_file_transfer_id_t, tego_bool_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(file_transfer_request_response_received, tego_user_id_t*, tego_file_transfer_id_t, tego_file_transfer_response_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(file_transfer_progress, tego_user_id_t*, tego_file_transfer_id_t, tego_file_transfer_direction_t, uint64_t, uint64_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(file_transfer_complete, tego_user_id_t*, tego_file_transfer_id_t, tego_file_transfer_direction_t, tego_file_transfer_result_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(user_status_changed, tego_user_id_t*, tego_user_status_t);
        TEGO_IMPLEMENT_CALLBACK_FUNCTIONS(new_identity_created, tego_ed25519_private_key_t*);


    private:
        void push_back(type_erased_callback&&);
        tego_context* context_ = nullptr;

        // cleanup message buffer
        static void cleanup_arg(char* msg)
        {
            delete[] msg;
        }

        // cleanup for other pointer types
        template<typename T>
        static void cleanup_arg(T* pVal)
        {
            delete pVal;
        }

        // no-op for unspecialized types
        template<typename T>
        static void cleanup_arg(T&&)
        { }

        // no-op for termation case
        static void cleanup_args()
        { }

        template<typename FIRST, typename...ARGS>
        static void cleanup_args(FIRST&& first, ARGS&&... args)
        {
            cleanup_arg(std::forward<FIRST>(first));
            return cleanup_args(std::forward<ARGS>(args)...);
        }
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
        // this queue is protected by mutex_ within worker_ thread and callback_queue methods
        std::vector<type_erased_callback> pending_callbacks_;

		// worker thread must be last so that other members are init'd before thread runs
        std::thread worker_;
    };
}
