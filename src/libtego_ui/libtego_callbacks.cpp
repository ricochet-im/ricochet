#include "utils/Settings.h"
#include "shims/TorControl.h"
#include "shims/TorManager.h"
#include "shims/UserIdentity.h"
#include "shims/ConversationModel.h"
#include "shims/OutgoingContactRequest.h"

namespace
{
    constexpr int consumeInterval = 10;

    // this holds a callback which can be called and then deletes the underlying data
    // replaces std::function beause std::function cannot be move constructed >:[
    class run_once_task
    {
    public:
        run_once_task() = default;
        run_once_task(run_once_task&& that)
        : run_once_task()
        {
            *this = std::move(that);
        }

        // no copying allowed
        run_once_task(const run_once_task&) = delete;
        run_once_task& operator=(run_once_task const&) = delete;

        // ensure move does not overwrite existing callback data
        run_once_task& operator=(run_once_task&& that)
        {
            Q_ASSERT(exec == nullptr && callable == nullptr);
            exec = that.exec;
            callable = that.callable;

            that.exec = nullptr;
            that.callable = nullptr;

            return *this;
        }

        template<typename LAMBDA>
        run_once_task(LAMBDA&& lambda)
        {
            // convertible to raw ptr
            if constexpr (std::is_convertible<LAMBDA, void(*)(void)>::value)
            {
                exec = [](run_once_task* self) -> void
                {
                    auto lambda = reinterpret_cast<void(*)(void)>(self->callable);
                    lambda();

                    self->exec = nullptr;
                    self->callable = nullptr;
                };
                callable = reinterpret_cast<void*>(static_cast<void(*)(void)>(lambda));
            }
            // otherwise make a heap copy
            else
            {
                exec = [](run_once_task* self) -> void
                {
                    auto lambda = reinterpret_cast<LAMBDA*>(self->callable);
                    (*lambda)();
                    delete lambda;

                    self->exec = nullptr;
                    self->callable = nullptr;
                };
                callable = new LAMBDA(std::move(lambda));
            }
        }

        // just ensure we're not messing anything up
        ~run_once_task()
        {
            Q_ASSERT(exec == nullptr && callable == nullptr);
        }


        void operator()()
        {
            exec(this);
        }

    private:
        void(*exec)(run_once_task*) = nullptr;
        void* callable = nullptr;
    };

    // data
    std::vector<run_once_task> taskQueue;
    std::mutex taskQueueLock;

    void consume_tasks()
    {
        // get sole access to the task queue
        static decltype(taskQueue) localTaskQueue;
        {
            std::lock_guard<std::mutex> lock(taskQueueLock);
            std::swap(taskQueue, localTaskQueue);
        }

        // consume all of our tasks
        for(auto& task : localTaskQueue)
        {
            try
            {
                task();
            }
            catch(std::exception& ex)
            {
                qDebug() << "Exception thrown from task: " << ex.what();
            }
        }

        // clear out our queue
        localTaskQueue.clear();

		// schedule us to run again
	    QTimer::singleShot(consumeInterval, &consume_tasks);
    }

    template<typename FUNC>
    void push_task(FUNC&& func)
    {
        // acquire lock on the queue and push our received functor
        std::lock_guard<std::mutex> lock(taskQueueLock);
        taskQueue.push_back(std::move(func));
    }

    QString serviceIdToContactId(const QString& serviceId)
    {
        return QStringLiteral("ricochet:%1").arg(serviceId);
    }

    QString tegoUserIdToServiceId(const tego_user_id_t* user)
    {
        std::unique_ptr<tego_v3_onion_service_id> serviceId;
        tego_user_id_get_v3_onion_service_id(user, tego::out(serviceId), tego::throw_on_error());

        char serviceIdRaw[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
        tego_v3_onion_service_id_to_string(serviceId.get(), serviceIdRaw, sizeof(serviceIdRaw), tego::throw_on_error());

        auto contactId = QString::fromUtf8(serviceIdRaw, TEGO_V3_ONION_SERVICE_ID_LENGTH);
        return contactId;
    }

    // converts the our tego_user_id_t to ricochet's contactId in the form ricochet:serviceidserviceidserviceid...
    QString tegoUserIdToContactId(const tego_user_id_t* user)
    {
        return serviceIdToContactId(tegoUserIdToServiceId(user));
    }

    shims::ContactUser* contactUserFromContactId(const QString& contactId)
    {
        auto userIdentity = shims::UserIdentity::userIdentity;
        auto contactsManager = userIdentity->getContacts();

        auto contactUser = contactsManager->getShimContactByContactId(contactId);
        return contactUser;
    }

    //
    // libtego callbacks
    //

    void on_tor_error_occurred(
        tego_context_t*,
        tego_tor_error_origin_t origin,
        const tego_error_t* error)
    {
        // route the error message to the appropriate component
        QString errorMsg = tego_error_get_message(error);
        logger::println("tor error : {}", errorMsg);
        push_task([=]() -> void
        {
            switch(origin)
            {
                case tego_tor_error_origin_control:
                {
                    shims::TorControl::torControl->setErrorMessage(errorMsg);
                }
                break;
                case tego_tor_error_origin_manager:
                {
                    shims::TorManager::torManager->setErrorMessage(errorMsg);
                }
                break;
            }
        });
    }

    void on_update_tor_daemon_config_succeeded(
        tego_context_t*,
        tego_bool_t success)
    {
        push_task([=]() -> void
        {
            logger::println("tor daemon config succeeded : {}", success);
            auto torControl = shims::TorControl::torControl;
            if (torControl->m_setConfigurationCommand != nullptr)
            {
                torControl->m_setConfigurationCommand->onFinished(success);
                torControl->m_setConfigurationCommand = nullptr;
            }
        });
    }

    void on_tor_control_status_changed(
        tego_context_t*,
        tego_tor_control_status_t status)
    {
        push_task([=]() -> void
        {
            logger::println("new control status : {}", status);
            shims::TorControl::torControl->setStatus(static_cast<shims::TorControl::Status>(status));
        });
    }

    void on_tor_process_status_changed(
        tego_context_t*,
        tego_tor_process_status_t status)
    {
        push_task([=]() -> void
        {
            logger::println("new process status : {}", status);
            auto torManager = shims::TorManager::torManager;
            switch(status)
            {
                case tego_tor_process_status_running:
                    torManager->setRunning("Yes");
                    break;
                case tego_tor_process_status_external:
                    torManager->setRunning("External");
                    break;
                default:
                    torManager->setRunning("No");
                    break;
            }
        });
    }

    void on_tor_network_status_changed(
        tego_context_t*,
        tego_tor_network_status_t status)
    {
        push_task([=]() -> void
        {
            logger::println("new network status : {}", status);
            auto torControl = shims::TorControl::torControl;
            switch(status)
            {
                case tego_tor_network_status_unknown:
                    torControl->setTorStatus(shims::TorControl::TorUnknown);
                    break;
                case tego_tor_network_status_ready:
                    torControl->setTorStatus(shims::TorControl::TorReady);
                    break;
                case tego_tor_network_status_offline:
                    torControl->setTorStatus(shims::TorControl::TorOffline);
                    break;
            }
        });
    }

    void on_tor_bootstrap_status_changed(
        tego_context_t*,
        int32_t progress,
        tego_tor_bootstrap_tag_t tag)
    {
        push_task([=]() -> void
        {
            logger::println("bootstrap status : {{ progress : {}, tag : {} }}", progress, (int)tag);
            auto torControl = shims::TorControl::torControl;
            emit torControl->bootstrapStatusChanged();
        });
    }

    void on_tor_log_received(
        tego_context_t*,
        const char* message,
        size_t messageLength)
    {
        auto messageString = QString::fromUtf8(message, messageLength);
        push_task([=]()-> void
        {
            auto torManager = shims::TorManager::torManager;
            emit torManager->logMessage(messageString);
        });
    }

    void on_host_user_state_changed(
        tego_context_t*,
        tego_host_user_state_t state)
    {
        logger::println("new host user state : {}", state);
        push_task([=]() -> void
        {
            auto userIdentity = shims::UserIdentity::userIdentity;
            switch(state)
            {
                case tego_host_user_state_offline:
                    userIdentity->setOnline(false);
                    break;
                case tego_host_user_state_online:
                    userIdentity->setOnline(true);
                    break;
                default:
                    break;
            }
        });
    }

    void on_chat_request_received(
        tego_context_t*,
        const tego_user_id_t* userId,
        const char* message,
        size_t messageLength)
    {
        logger::println("Received chat request from {}", tegoUserIdToServiceId(userId));
        logger::println("Message : {}", message);

        auto hostname = tegoUserIdToServiceId(userId) + ".onion";
        auto messageString = QString::fromUtf8(message, messageLength);

        push_task([=]() -> void
        {
            auto userIdentity = shims::UserIdentity::userIdentity;
            userIdentity->createIncomingContactRequest(hostname, messageString);
        });
    }

    void on_chat_request_response_received(
        tego_context_t*,
        const tego_user_id_t* userId,
        tego_bool_t requestAccepted)
    {
        logger::trace();

        auto serviceId = tegoUserIdToServiceId(userId);

        push_task([=]() -> void
        {
            auto userIdentity = shims::UserIdentity::userIdentity;
            auto contactsManager = userIdentity->getContacts();
            auto contact = contactsManager->getShimContactByContactId(serviceIdToContactId(serviceId));
            auto outgoingContactRequest = contact->contactRequest();

            logger::trace();

            if (requestAccepted)
            {
                outgoingContactRequest->setAccepted();
            }
            else
            {
                outgoingContactRequest->setRejected();
                contact->setStatus(shims::ContactUser::RequestRejected);
            }
        });
    }

    void on_user_status_changed(
        tego_context_t*,
        const tego_user_id_t* userId,
        tego_user_status_t status)
    {
        logger::trace();
        auto serviceId = tegoUserIdToServiceId(userId);

        logger::println("user status changed -> service id : {}, status : {}", serviceId, (int)status);

        push_task([=]() -> void
        {
            auto userIdentity = shims::UserIdentity::userIdentity;
            auto contactsManager = userIdentity->getContacts();
            auto contact = contactsManager->getShimContactByContactId(serviceIdToContactId(serviceId));
            auto conversation = contact->conversation();

            if (contact != nullptr)
            {
                switch(status)
                {
                    case tego_user_status_online:
                        contact->setStatus(shims::ContactUser::Online);
                        contactsManager->setContactStatus(contact, shims::ContactUser::Online);
                        conversation->setStatus(shims::ContactUser::Online);
                        break;
                    case tego_user_status_offline:
                        contact->setStatus(shims::ContactUser::Offline);
                        contactsManager->setContactStatus(contact, shims::ContactUser::Offline);
                        conversation->setStatus(shims::ContactUser::Offline);
                        break;
                    default:
                        break;
                }
            }
        });
    }

    void on_message_received(
        tego_context_t*,
        const tego_user_id_t* sender,
        tego_time_t timestamp,
        tego_message_id_t messageId,
        const char* message,
        size_t messageLength)
    {
        auto contactId = tegoUserIdToContactId(sender);
        auto messageString = QString::fromUtf8(message, messageLength);

        push_task([=]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->messageReceived(messageId, QDateTime::fromMSecsSinceEpoch(timestamp), messageString);
        });
    }

    void on_message_acknowledged(
        tego_context_t*,
        const tego_user_id_t* userId,
        tego_message_id_t messageId,
        tego_bool_t messageAccepted)
    {
        logger::trace();
        logger::println(" userId : {}", (void*)userId);
        logger::println(" messageId : {}", messageId);
        logger::println(" messageAccepted : {}", messageAccepted);

        QString contactId = tegoUserIdToContactId(userId);
        push_task([=]() -> void
        {
            logger::trace();
            auto contactsManager = shims::UserIdentity::userIdentity->getContacts();
            auto contactUser = contactsManager->getShimContactByContactId(contactId);
            auto conversationModel = contactUser->conversation();
            conversationModel->messageAcknowledged(messageId, static_cast<bool>(messageAccepted));
        });
    }

    void on_file_transfer_request_received(
        tego_context_t*,
        tego_user_id_t const* sender,
        tego_file_transfer_id_t id,
        char const* fileName,
        size_t fileNameLength,
        tego_file_size_t fileSize,
        tego_file_hash_t const* fileHash)
    {
        auto contactId = tegoUserIdToContactId(sender);
        QString fileNameCopy = QString::fromUtf8(fileName, fileNameLength);
        auto hashStr = tego::to_string(fileHash);

        push_task([=,fileName=std::move(fileNameCopy)]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->fileTransferRequestReceived(id, fileName, QString::fromStdString(hashStr), fileSize);
        });
    }

    void on_file_transfer_request_acknowledged(
        tego_context_t*,
        tego_user_id_t const* receiver,
        tego_file_transfer_id_t id,
        tego_bool_t ack)
    {
        auto contactId = tegoUserIdToContactId(receiver);

        push_task([=]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->fileTransferRequestAcknowledged(id, ack);
        });
    }

    void on_file_transfer_request_response_received(
        tego_context_t*,
        tego_user_id_t const* receiver,
        tego_file_transfer_id_t id,
        tego_file_transfer_response_t response)
    {
        auto contactId = tegoUserIdToContactId(receiver);

        push_task([=]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->fileTransferRequestResponded(id, response);
        });
    }

    void on_file_transfer_progress(
        tego_context_t*,
        const tego_user_id_t* userId,
        tego_file_transfer_id_t id,
        tego_file_transfer_direction_t direction,
        tego_file_size_t bytesComplete,
        tego_file_size_t bytesTotal)
    {
        auto contactId = tegoUserIdToContactId(userId);

        push_task([=]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->fileTransferRequestProgressUpdated(id, bytesComplete);
        });


        logger::println(
            "File Progress id : {}, direction : {}, transferred : {} bytes, total : {} bytes",
            id,
            direction == tego_file_transfer_direction_sending ? "sending" : "receiving",
            bytesComplete,
            bytesTotal);
    }

    void on_file_transfer_complete(
        tego_context_t*,
        const tego_user_id_t* userId,
        tego_file_transfer_id_t id,
        tego_file_transfer_direction_t,
        tego_file_transfer_result_t result)
    {
        auto contactId = tegoUserIdToContactId(userId);

        push_task([=]() -> void
        {
            auto contactUser = contactUserFromContactId(contactId);
            Q_ASSERT(contactUser != nullptr);
            auto conversationModel = contactUser->conversation();
            Q_ASSERT(conversationModel != nullptr);

            conversationModel->fileTransferRequestCompleted(id, result);
        });
    }

    void on_new_identity_created(
        tego_context_t*,
        const tego_ed25519_private_key_t* privateKey)
    {
        // convert privateKey to KeyBlob
        char rawKeyBlob[TEGO_ED25519_KEYBLOB_SIZE] = {0};
        tego_ed25519_keyblob_from_ed25519_private_key(
            rawKeyBlob,
            sizeof(rawKeyBlob),
            privateKey,
            tego::throw_on_error());

        QString keyBlob(rawKeyBlob);

        push_task([=]() -> void
        {
            SettingsObject so(QStringLiteral("identity"));
            so.write("privateKey", keyBlob);
        });
    }
}

void init_libtego_callbacks(tego_context_t* context)
{
    // start triggering our consume queue
    QTimer::singleShot(consumeInterval, &consume_tasks);

    //
    // register each of our callbacks with libtego
    //

    tego_context_set_tor_error_occurred_callback(
        context,
        &on_tor_error_occurred,
        tego::throw_on_error());

    tego_context_set_update_tor_daemon_config_succeeded_callback(
        context,
        &on_update_tor_daemon_config_succeeded,
        tego::throw_on_error());

    tego_context_set_tor_control_status_changed_callback(
        context,
        &on_tor_control_status_changed,
        tego::throw_on_error());

    tego_context_set_tor_process_status_changed_callback(
        context,
        &on_tor_process_status_changed,
        tego::throw_on_error());

    tego_context_set_tor_network_status_changed_callback(
        context,
        &on_tor_network_status_changed,
        tego::throw_on_error());

    tego_context_set_tor_bootstrap_status_changed_callback(
        context,
        &on_tor_bootstrap_status_changed,
        tego::throw_on_error());

    tego_context_set_tor_log_received_callback(
        context,
        &on_tor_log_received,
        tego::throw_on_error());

    tego_context_set_host_user_state_changed_callback(
        context,
        &on_host_user_state_changed,
        tego::throw_on_error());

    tego_context_set_chat_request_received_callback(
        context,
        &on_chat_request_received,
        tego::throw_on_error());

    tego_context_set_chat_request_response_received_callback(
        context,
        &on_chat_request_response_received,
        tego::throw_on_error());

    tego_context_set_file_transfer_request_received_callback(
        context,
        &on_file_transfer_request_received,
        tego::throw_on_error());

    tego_context_set_file_transfer_request_acknowledged_callback(
        context,
        &on_file_transfer_request_acknowledged,
        tego::throw_on_error());

    tego_context_set_file_transfer_request_response_received_callback(
        context,
        &on_file_transfer_request_response_received,
        tego::throw_on_error());

    tego_context_set_file_transfer_progress_callback(
        context,
        &on_file_transfer_progress,
        tego::throw_on_error());

    tego_context_set_file_transfer_complete_callback(
        context,
        &on_file_transfer_complete,
        tego::throw_on_error());

    tego_context_set_user_status_changed_callback(
        context,
        &on_user_status_changed,
        tego::throw_on_error());

    tego_context_set_message_received_callback(
        context,
        &on_message_received,
        tego::throw_on_error());

    tego_context_set_message_acknowledged_callback(
        context,
        &on_message_acknowledged,
        tego::throw_on_error());

    tego_context_set_new_identity_created_callback(
        context,
        &on_new_identity_created,
        tego::throw_on_error());
}