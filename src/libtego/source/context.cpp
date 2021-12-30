#include "context.hpp"
#include "error.hpp"
#include "globals.hpp"
#include "tor.hpp"
#include "user.hpp"
#include "ed25519.hpp"

using tego::g_globals;

#include "tor/TorControl.h"
#include "tor/TorManager.h"
#include "tor/TorProcess.h"
#include "core/UserIdentity.h"
#include "core/ContactUser.h"
#include "core/ConversationModel.h"

//
// Tego Context
//

tego_context::tego_context()
: callback_registry_(this)
, callback_queue_(this)
, threadId(std::this_thread::get_id())
{
    this->torManager = Tor::TorManager::instance();
    this->torControl = torManager->control();
}

void tego_context::start_tor(const tego_tor_launch_config_t* config)
{
    TEGO_THROW_IF_NULL(this->torManager);
    TEGO_THROW_IF_NULL(config);

    this->torManager->setDataDirectory(config->dataDirectory.data());
    this->torManager->start();
}

bool tego_context::get_tor_daemon_configured() const
{
    TEGO_THROW_IF_NULL(this->torManager);

    return !this->torManager->configurationNeeded();
}

size_t tego_context::get_tor_logs_size() const
{
    size_t retval = 0;
    for(const auto& msg : this->get_tor_logs())
    {
        retval += msg.size() + 1;
    }
    return retval;
}

const std::vector<std::string>& tego_context::get_tor_logs() const
{
    TEGO_THROW_IF_NULL(this->torManager);

    auto logMessages = this->torManager->logMessages();
    TEGO_THROW_IF_FALSE(logMessages.size() >= 0);

    // see if we need to update our local copy
    if(static_cast<size_t>(logMessages.size()) != this->torLogs.size())
    {
        for(size_t i = this->torLogs.size(); i < static_cast<size_t>(logMessages.size()); i++)
        {
            TEGO_THROW_IF_FALSE(i < std::numeric_limits<int>::max());
            this->torLogs.push_back(logMessages[static_cast<int>(i)].toStdString());
        }
    }

    return this->torLogs;
}

const char* tego_context::get_tor_version_string() const
{
    if (torVersion.empty())
    {
        TEGO_THROW_IF_NULL(this->torControl);

        QString version = this->torControl->torVersion();
        this->torVersion = version.toStdString();
    }

    return this->torVersion.c_str();
}

tego_tor_control_status_t tego_context::get_tor_control_status() const
{
    TEGO_THROW_IF_NULL(this->torControl);
    return static_cast<tego_tor_control_status_t>(this->torControl->status());
}

tego_tor_process_status_t tego_context::get_tor_process_status() const
{
    TEGO_THROW_IF_NULL(this->torManager);

    auto torProcess = this->torManager->process();
    if (torProcess == nullptr)
    {
        return tego_tor_process_status_external;
    }

    switch(torProcess->state())
    {
        case Tor::TorProcess::Failed:
            return tego_tor_process_status_failed;
        case Tor::TorProcess::NotStarted:
            return tego_tor_process_status_not_started;
        case Tor::TorProcess::Starting:
            return tego_tor_process_status_starting;
        case Tor::TorProcess::Connecting:
            // fall through, Connecting is really control status and not used by frontend explicitly
        case Tor::TorProcess::Ready:
            return tego_tor_process_status_running;
    }

    return tego_tor_process_status_unknown;
}

tego_tor_network_status_t tego_context::get_tor_network_status() const
{
    TEGO_THROW_IF_NULL(this->torControl);
    switch(this->torControl->torStatus())
    {
        case Tor::TorControl::TorOffline:
            return tego_tor_network_status_offline;
        case Tor::TorControl::TorReady:
            return tego_tor_network_status_ready;
        default:
            return tego_tor_network_status_unknown;
    }
}

tego_tor_bootstrap_tag_t tego_context::get_tor_bootstrap_tag() const
{
    TEGO_THROW_IF_NULL(this->torControl);
    auto bootstrapStatus = this->torControl->bootstrapStatus();
    auto bootstrapTag = bootstrapStatus["tag"].toString();

    // see https://gitweb.torproject.org/torspec.git/tree/control-spec.txt#n3867
    // TODO: optimize this function if you're bored, could do binary search rather than linear search
    constexpr static const char* tagList[] =
    {
        "starting",
        "conn_pt",
        "conn_done_pt",
        "conn_proxy",
        "conn_done_proxy",
        "conn",
        "conn_done",
        "handshake",
        "handshake_done",
        "onehop_create",
        "requesting_status",
        "loading_status",
        "loading_keys",
        "requesting_descriptors",
        "loading_descriptors",
        "enough_dirinfo",
        "ap_conn_pt_summary",
        "ap_conn_done_pt",
        "ap_conn_proxy",
        "ap_conn_done_proxy",
        "ap_conn",
        "ap_conn_done",
        "ap_handshake",
        "ap_handshake_done",
        "circuit_create",
        "done",
    };
    static_assert(tego::countof(tagList) == tego_tor_bootstrap_tag_count);

    for(size_t i = 0; i < tego_tor_bootstrap_tag_count; i++)
    {
        if (tagList[i] == bootstrapTag)
        {
            return static_cast<tego_tor_bootstrap_tag_t>(i);
        }
    }

    TEGO_THROW_MSG("unrecognized bootstrap tag : \"{}\"", bootstrapTag);
}

void tego_context::start_service(
    tego_ed25519_private_key_t const* hostPrivateKey,
    tego_user_id_t const* const* userBuffer,
    tego_user_type_t* const userTypeBuffer,
    size_t userCount)
{
    TEGO_THROW_IF_NULL(hostPrivateKey);
    if (userCount > 0)
    {
        TEGO_THROW_IF_NULL(userBuffer);
        TEGO_THROW_IF_NULL(userTypeBuffer);
    }
    else
    {
        TEGO_THROW_IF_NOT_NULL(userBuffer);
        TEGO_THROW_IF_NOT_NULL(userTypeBuffer);
    }

    char rawKeyBlob[TEGO_ED25519_KEYBLOB_SIZE] = {0};
    tego_ed25519_keyblob_from_ed25519_private_key(
        rawKeyBlob,
        sizeof(rawKeyBlob),
        hostPrivateKey,
        tego::throw_on_error());

    auto keyBlob = QString::fromUtf8(rawKeyBlob, TEGO_ED25519_KEYBLOB_LENGTH);

    // our different types of users
    QList<QString> allowedUsers;
    QList<QString> requestingUsers;
    QList<QString> blockedUsers;
    QList<QString> pendingUsers;
    QList<QString> rejectedUsers;

    for(size_t k = 0; k < userCount; k++)
    {
        const auto userType = userTypeBuffer[k];
        const auto userHostname = QString::fromUtf8(userBuffer[k]->serviceId.data, TEGO_V3_ONION_SERVICE_ID_LENGTH) + ".onion";

        switch(userTypeBuffer[k])
        {
            case tego_user_type_host:
                TEGO_THROW_MSG("passed in userTypeBuffer[{}] is invalid type 'tego_user_type_host'", k);
                break;
            case tego_user_type_allowed:
                allowedUsers.push_back(userHostname);
                break;
            case tego_user_type_requesting:
                requestingUsers.push_back(userHostname);
                break;
            case tego_user_type_blocked:
                blockedUsers.push_back(userHostname);
                break;
            case tego_user_type_pending:
                pendingUsers.push_back(userHostname);
                break;
            case tego_user_type_rejected:
                rejectedUsers.push_back(userHostname);
                break;
            default:
                TEGO_THROW_MSG("passed in userTypeBuffer[{}] : ({}) is invalid", k, static_cast<int>(userType));
                break;
        }
    }

    // save off the singleton on our context
    this->identityManager = new IdentityManager(keyBlob);
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();

    contactsManager->addAllowedContacts(allowedUsers);
    contactsManager->addIncomingRequests(requestingUsers);
    contactsManager->addRejectedIncomingRequests(blockedUsers);
    contactsManager->addOutgoingRequests(pendingUsers);
    contactsManager->addRejectedOutgoingRequests(rejectedUsers);
}

void tego_context::start_service()
{
    this->identityManager = new IdentityManager({}, {});
}

int32_t tego_context::get_tor_bootstrap_progress() const
{
    TEGO_THROW_IF_NULL(this->torControl);
    auto bootstrapStatus = this->torControl->bootstrapStatus();
    auto bootstrapProgress = bootstrapStatus["progress"].toInt();
    return static_cast<int32_t>(bootstrapProgress);
}

void tego_context::update_tor_daemon_config(const tego_tor_daemon_config_t* daemonConfig)
{
    TEGO_THROW_IF_NULL(this->torControl);

    const auto& config = *daemonConfig;

    QVariantMap vm;

    // init the tor settings we can modify here
    constexpr static auto configKeys =
    {
        "DisableNetwork",
        "Socks4Proxy",
        "Socks5Proxy",
        "Socks5ProxyUsername",
        "Socks5ProxyPassword",
        "HTTPSProxy",
        "HTTPSProxyAuthenticator",
        "ReachableAddresses",
        "Bridge",
        "UseBridges",
    };
    for(const auto& currentKey : configKeys)
    {
        vm[currentKey] = "";
    }

    // set disable network flag
    if (config.disableNetwork.has_value()) {
        vm["DisableNetwork"] = (config.disableNetwork.value() ? "1" : "0");
    }

    // set proxy info
    switch(config.proxy.type)
    {
        case tego_proxy_type_none: break;
        case tego_proxy_type_socks4:
        {
            vm["Socks4Proxy"] = QString::fromStdString(fmt::format("{}:{}", config.proxy.address, config.proxy.port));
        }
        break;
        case tego_proxy_type_socks5:
        {
            vm["Socks5Proxy"] = QString::fromStdString(fmt::format("{}:{}", config.proxy.address, config.proxy.port));
            if (!config.proxy.username.empty())
            {
                vm["Socks5ProxyUsername"] = QString::fromStdString(config.proxy.username);
            }
            if (!config.proxy.password.empty())
            {
                vm["Socks5ProxyPassword"] = QString::fromStdString(config.proxy.password);
            }
        }
        break;
        case tego_proxy_type_https:
        {
            vm["HTTPSProxy"] = QString::fromStdString(fmt::format("{}:{}", config.proxy.address, config.proxy.port));
            if (!config.proxy.username.empty() || !config.proxy.password.empty())
            {
                vm["HTTPSProxyAuthenticator"] = QString::fromStdString(fmt::format("{}:{}", config.proxy.username, config.proxy.password));
            }
        }
        break;
    }

    // set firewall ports
    if (config.allowedPorts.size() > 0)
    {
        std::stringstream ss;

        auto it = config.allowedPorts.begin();

        ss << fmt::format("*:{}", *it);
        for(++it; it < config.allowedPorts.end(); ++it)
        {
            ss << fmt::format(", *:{}", *it);
        }

        vm["ReachableAddresses"] = QString::fromStdString(ss.str());
    }

    // set bridges
    if (config.bridges.size() > 0)
    {
        QVariantList bridges;
        for(const auto& currentBridge : config.bridges)
        {
            bridges.append(QString::fromStdString(currentBridge));
        }
        vm["Bridge"] = bridges;
        vm["UseBridges"] = "1";
    }

    this->torControl->setConfiguration(vm);
}

void tego_context::save_tor_daemon_config()
{
    TEGO_THROW_IF_NULL(this->torControl);
    logger::trace();
    this->torControl->saveConfiguration();
}

void tego_context::set_host_user_state(tego_host_user_state_t state)
{
    if (state == hostUserState)
    {
        return;
    }

    this->hostUserState = state;
    this->callback_registry_.emit_host_user_state_changed(state);
}

std::unique_ptr<tego_user_id_t> tego_context::get_host_user_id() const
{
    TEGO_THROW_IF_NULL(this->identityManager);
    auto userIdentity = this->identityManager->identities().first();

    auto hostname = userIdentity->hostname().toUtf8();
    tego_v3_onion_service_id serviceId(hostname.data(), TEGO_V3_ONION_SERVICE_ID_LENGTH);

    return std::make_unique<tego_user_id_t>(serviceId);
}

tego_host_user_state_t tego_context::get_host_user_state() const
{
    return this->hostUserState;
}

void tego_context::send_chat_request(
    const tego_user_id_t* user,
    const char* message,
    size_t messageLength)
{
    TEGO_THROW_IF_NULL(this->identityManager);
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();

    TEGO_THROW_IF_FALSE(messageLength < std::numeric_limits<int>::max());
    contactsManager->createContactRequest(
        QString::fromStdString(fmt::format("ricochet:{}", user->serviceId.data)),
        (messageLength == 0) ? QString() : QString::fromUtf8(message, static_cast<int>(messageLength)));
}

void tego_context::acknowledge_chat_request(
        const tego_user_id_t* user,
        tego_chat_acknowledge_t response)
{
    TEGO_THROW_IF_NULL(user);

    logger::println("ack chat request from {}", user->serviceId.data);
    logger::println("response : {}", static_cast<int>(response));

    TEGO_THROW_IF_NULL(this->identityManager);
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();
    auto incomingRequestManager = contactsManager->incomingRequestManager();

    auto hostname = QString("%1.onion").arg(user->serviceId.data).toUtf8();

    auto incomingContactRequest = incomingRequestManager->requestFromHostname(hostname);
    TEGO_THROW_IF_NULL(incomingContactRequest);

    switch(response)
    {
        case tego_chat_acknowledge_accept:
            incomingContactRequest->accept(nullptr);
            break;
        case tego_chat_acknowledge_reject:
            incomingContactRequest->reject();
            break;
        case tego_chat_acknowledge_block:
            incomingContactRequest->reject();
            incomingRequestManager->addRejectedHost(hostname);
            break;
    }
}

tego_message_id_t tego_context::send_message(
    const tego_user_id_t* user,
    const std::string& message)
{
    TEGO_THROW_IF_NULL(user);
    TEGO_THROW_IF_FALSE(message.size() > 0)

    auto contactUser = getContactUser(user);
    TEGO_THROW_IF_NULL(contactUser);
    auto conversationModel = contactUser->conversation();

    return conversationModel->sendMessage(QString::fromStdString(message));
}

tego_user_type_t tego_context::get_user_type(tego_user_id_t const* user) const
{
    auto contactUser = this->getContactUser(user);
    if (contactUser != nullptr)
    {
        auto const status = contactUser->status();
        switch(status)
        {
            case ContactUser::Online:
            case ContactUser::Offline:
                return tego_user_type_allowed;
            case ContactUser::RequestPending:
                return tego_user_type_pending;
            case ContactUser::RequestRejected:
                return tego_user_type_rejected;
            default:
                TEGO_THROW_MSG("Unknown ContactUser::Status : {}", static_cast<int>(status));
                break;
        }
    }

    // next check for requesting users
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();
    auto incomingRequestManager = contactsManager->incomingRequestManager();

    auto hostname = QByteArray::fromStdString(fmt::format("{}.onion", user->serviceId.data));

    // search for the user's serviceId on the incoming request manager
    auto contactRequest = incomingRequestManager->requestFromHostname(hostname);
    if (contactRequest != nullptr)
    {
        return tego_user_type_requesting;
    }

    // see if the user is blocked
    if(incomingRequestManager->isHostnameRejected(hostname))
    {
        return tego_user_type_blocked;
    }

    // finally see if the user matches the host
    if (hostname == userIdentity->hostname().toUtf8())
    {
        return tego_user_type_host;
    }

    TEGO_THROW_MSG("Unknown user with service id : '{}'", user->serviceId.data);
}

size_t tego_context::get_user_count() const
{
    TEGO_THROW_IF_NULL(this->identityManager);
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();

    return static_cast<size_t>(contactsManager->contacts().size());
}

std::vector<tego_user_id_t*> tego_context::get_users() const
{
    TEGO_THROW_IF_NULL(this->identityManager);
    auto userIdentity = this->identityManager->identities().first();
    auto contactsManager = userIdentity->getContacts();
    auto incomingRequestManager = contactsManager->incomingRequestManager();

    std::vector<std::unique_ptr<tego_user_id_t>> managedUsers;
    std::vector<tego_user_id_t*> users;

    // helper function to from hostname to tego_user_id_t
    auto hostnameToTegoUserId = [](QString const& hostname) -> std::unique_ptr<tego_user_id_t>
    {
         // convert our hostname to just the service id raw string
        auto serviceIdString = hostname.left(TEGO_V3_ONION_SERVICE_ID_LENGTH).toUtf8();
        // ensure valid service id
        auto serviceId = std::make_unique<tego_v3_onion_service_id>(serviceIdString.data(), serviceIdString.size());
        // create user id object from service id
        auto userId = std::make_unique<tego_user_id>(*serviceId.get());

        return userId;
    };

    // first iterate through our explicit users
    for(auto contactUser : contactsManager->contacts())
    {
        auto userId = hostnameToTegoUserId(contactUser->hostname());

        users.push_back(userId.get());
        managedUsers.push_back(std::move(userId));
    }

    // next add our implicit 'incomingContactRequest' users
    for(auto incomingContactRequest : incomingRequestManager->requests())
    {
        auto userId = hostnameToTegoUserId(incomingContactRequest->hostname());

        users.push_back(userId.get());
        managedUsers.push_back(std::move(userId));
    }

    // next add our blocked users
    for(auto rejectedHostname : incomingRequestManager->getRejectedHostnames())
    {
        auto userId = hostnameToTegoUserId(rejectedHostname);

        users.push_back(userId.get());
        managedUsers.push_back(std::move(userId));
    }

    // we got this far, release these ptrs from memory management
    for(auto& mu : managedUsers)
    {
        mu.release();
    }

    return users;
}

void tego_context::forget_user(const tego_user_id_t* user)
{
    // TODO: does not handle our blocked users or incoming request users
    auto contactUser = this->getContactUser(user);
    TEGO_THROW_IF_NULL(contactUser);
    contactUser->deleteContact();
}

std::tuple<tego_file_transfer_id_t, std::unique_ptr<tego_file_hash_t>, tego_file_size_t> tego_context::send_file_transfer_request(
    tego_user_id_t const* user,
    std::string const& filePath)
{
    TEGO_THROW_IF_NULL(user);

    auto contactUser = this->getContactUser(user);
    TEGO_THROW_IF_NULL(contactUser);
    auto conversationModel = contactUser->conversation();

    return conversationModel->sendFile(QString::fromStdString(filePath));
}

void tego_context::respond_file_transfer_request(
    tego_user_id_t const* user,
    tego_file_transfer_id_t fileTransfer,
    tego_file_transfer_response_t response,
    std::string const& destPath)
{
    // ensure we have a valid user
    TEGO_THROW_IF_NULL(user);
    // ensure a valid response
    TEGO_THROW_IF_FALSE(response == tego_file_transfer_response_accept ||
                        response == tego_file_transfer_response_reject);
    // ensure non-empty dest path in case we are accepting
    TEGO_THROW_IF_TRUE(response == tego_file_transfer_response_accept && destPath.empty())

    auto contactUser = this->getContactUser(user);
    TEGO_THROW_IF_NULL(contactUser);
    auto conversationModel = contactUser->conversation();

    switch(response)
    {
        case tego_file_transfer_response_accept:
        {
            conversationModel->acceptFile(fileTransfer, destPath);
        }
        break;
        case tego_file_transfer_response_reject:
        {
            conversationModel->rejectFile(fileTransfer);
        }
        break;
    }
}

void tego_context::cancel_file_transfer_transfer(
    tego_user_id_t const* user,
    tego_file_transfer_id_t fileTransfer)
{
    // ensure we have a valid user
    TEGO_THROW_IF_NULL(user);

    auto contactUser = this->getContactUser(user);
    TEGO_THROW_IF_NULL(contactUser);
    auto conversationModel = contactUser->conversation();

    conversationModel->cancelTransfer(fileTransfer);
}

//
// tego_context private methods
//

ContactUser* tego_context::getContactUser(tego_user_id_t const* user) const
{
    TEGO_THROW_IF_NULL(user);
    TEGO_THROW_IF_NULL(identityManager);

    auto contactsManager = identityManager->identities().first()->getContacts();
    auto contactUser = contactsManager->lookupHostname(
        QString::fromUtf8(
            user->serviceId.data,
            TEGO_V3_ONION_SERVICE_ID_LENGTH));

    return contactUser;
}

//
// Exports
//

extern "C"
{
    // Bootstrap Tag

    const char* tego_tor_bootstrap_tag_to_summary(
        tego_tor_bootstrap_tag_t tag,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> const char*
        {
            TEGO_THROW_IF_FALSE(tag > tego_tor_bootstrap_tag_invalid &&
                                tag < tego_tor_bootstrap_tag_count);

            // strings from: https://gitweb.torproject.org/torspec.git/tree/control-spec.txt#n3867
            constexpr static const char* summaryList[] =
            {
                "Starting",
                "Connecting to pluggable transport",
                "Connected to pluggable transport",
                "Connecting to proxy",
                "Connected to proxy",
                "Connecting to a relay",
                "Connected to a relay",
                "Handshaking with a relay",
                "Handshake with a relay done",
                "Establishing an encrypted directory connection",
                "Asking for networkstatus consensus",
                "Loading networkstatus consensus",
                "Loading authority key certs",
                "Asking for relay descriptors",
                "Loading relay descriptors",
                "Loaded enough directory info to build circuits",
                "Connecting to pluggable transport to build circuits",
                "Connected to pluggable transport to build circuits",
                "Connecting to proxy to build circuits",
                "Connected to proxy to build circuits",
                "Connecting to a relay to build circuits",
                "Connected to a relay to build circuits",
                "Finishing handshake with a relay to build circuits",
                "Handshake finished with a relay to build circuits",
                "Establishing a Tor circuit",
                "Done",
            };
            static_assert(tego::countof(summaryList) == tego_tor_bootstrap_tag_count);

            return summaryList[static_cast<size_t>(tag)];
        }, error, nullptr);
    }

    // Tego Context

    void tego_context_start_tor(
        tego_context_t* context,
        const tego_tor_launch_config_t* launchConfig,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            context->start_tor(launchConfig);
        }, error);
    }

    void tego_context_get_tor_daemon_configured(
        const tego_context_t* context,
        tego_bool_t* out_configured,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_configured);

            *out_configured = TEGO_FALSE;

            if(context->get_tor_daemon_configured())
            {
                *out_configured = TEGO_TRUE;
            }
        }, error);
    }

    size_t tego_context_get_tor_logs_size(
        const tego_context_t* context,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> size_t
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            return context->get_tor_logs_size();
        }, error, 0);
    }


    size_t tego_context_get_tor_logs(
        const tego_context_t* context,
        char* out_logBuffer,
        size_t logBufferSize,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> size_t
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_logBuffer);

            // nothing to do if no space to write
            if (logBufferSize == 0)
            {
                return 0;
            }

            // get our tor logs
            const auto& logs = context->get_tor_logs();
            size_t logsSize = context->get_tor_logs_size();

            // create temporary buffer to copy each line into
            std::vector<char> logBuffer;
            logBuffer.reserve(logsSize);

            // copy each log and separate by new lines '\n'
            for(const auto& line : logs)
            {
                std::copy(line.begin(), line.end(), std::back_inserter(logBuffer));
                logBuffer.push_back('\n');
            }
            // append null terminator
            logBuffer.push_back(0);

            // finally copy at most logBufferSize bytes from logBuffer
            size_t copyCount = std::min(logBufferSize, logBuffer.size());
            TEGO_THROW_IF_FALSE(copyCount > 0);

            // check that we won't overflow
            TEGO_THROW_IF_FALSE(copyCount < std::numeric_limits<long>::max());

            std::copy(logBuffer.begin(), logBuffer.begin() + static_cast<long>(copyCount), out_logBuffer);
            // always write null terminator at the end
            out_logBuffer[copyCount - 1] = 0;

            return copyCount;
        }, error, 0);
    }

    const char* tego_context_get_tor_version_string(
        const tego_context_t* context,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> const char*
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            return context->get_tor_version_string();
        }, error, nullptr);
    }

    void tego_context_get_tor_control_status(
        const tego_context_t* context,
        tego_tor_control_status_t* out_status,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_status);

            auto status = context->get_tor_control_status();
            *out_status = status;
        }, error);
    }

    void tego_context_get_tor_process_status(
        const tego_context_t* context,
        tego_tor_process_status_t* out_status,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_status);

            const auto status = context->get_tor_process_status();
            *out_status = status;
        }, error);
    }

    void tego_context_get_tor_network_status(
        const tego_context_t* context,
        tego_tor_network_status_t* out_status,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_status);

            auto status = context->get_tor_network_status();
            *out_status = status;
        }, error);
    }

    void tego_context_get_tor_bootstrap_status(
        const tego_context* context,
        int32_t* out_progress,
        tego_tor_bootstrap_tag_t* out_tag,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_progress);
            TEGO_THROW_IF_NULL(out_tag);

            auto progress = context->get_tor_bootstrap_progress();
            auto tag = context->get_tor_bootstrap_tag();

            *out_progress = progress;
            *out_tag = tag;
        }, error);
    }

    void tego_context_start_service(
        tego_context_t* context,
        tego_ed25519_private_key_t const* hostPrivateKey,
        tego_user_id_t const* const* userBuffer,
        tego_user_type_t* const userTypeBuffer,
        size_t userCount,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            if (hostPrivateKey == nullptr)
            {
                TEGO_THROW_IF_FALSE(userBuffer == nullptr && userTypeBuffer == nullptr && userCount == 0);
                context->start_service();
            }
            else
            {
                TEGO_THROW_IF_FALSE((userBuffer == nullptr && userTypeBuffer == nullptr && userCount == 0) ||
                                    (userBuffer != nullptr && userTypeBuffer != nullptr && userCount > 0));

                context->start_service(
                    hostPrivateKey,
                    userBuffer,
                    userTypeBuffer,
                    userCount);
            }
        }, error);
    }

    void tego_context_get_host_user_id(
        const tego_context_t* context,
        tego_user_id_t** out_hostUser,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_hostUser);

            auto hostUser = context->get_host_user_id();
            *out_hostUser = hostUser.release();
        }, error);
    }

    void tego_context_get_host_user_state(
        const tego_context_t* context,
        tego_host_user_state_t* out_state,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_state);

            auto state = context->get_host_user_state();
            *out_state = state;
        }, error);
    }

    void tego_context_update_tor_daemon_config(
        tego_context_t* context,
        const tego_tor_daemon_config_t* torConfig,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            context->update_tor_daemon_config(torConfig);
        }, error);
    }

    void tego_context_save_tor_daemon_config(
        tego_context_t* context,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            context->save_tor_daemon_config();
        }, error);
    }

    void tego_context_get_user_type(
        const tego_context_t* context,
        const tego_user_id_t* user,
        tego_user_type_t* out_type,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            TEGO_THROW_IF_NULL(out_type);

            auto type = context->get_user_type(user);
            *out_type = type;
        }, error);
    }

    void tego_context_get_user_count(
        const tego_context_t* context,
        size_t* out_userCount,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_userCount);

            auto count = context->get_user_count();
            *out_userCount = count;
        }, error);
    }

    void tego_context_get_users(
        const tego_context_t* context,
        tego_user_id_t** out_usersBuffer,
        size_t usersBufferLength,
        size_t* out_userCount,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(out_usersBuffer);
            TEGO_THROW_IF_NULL(out_userCount);

            auto users = context->get_users();
            const auto userCount = std::min(users.size(), usersBufferLength);
            for(size_t i = 0; i < userCount; ++i)
            {
                out_usersBuffer[i] = users[i];
            }
            *out_userCount = userCount;

        }, error);
    }

    void tego_context_send_chat_request(
        tego_context_t* context,
        const tego_user_id_t* user,
        const char* message,
        size_t messageLength,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            TEGO_THROW_IF_FALSE(message != nullptr || messageLength == 0);

            context->send_chat_request(user, message, messageLength);
        }, error);
    }

    void tego_context_acknowledge_chat_request(
        tego_context_t* context,
        const tego_user_id_t* user,
        tego_chat_acknowledge_t response,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());

            context->acknowledge_chat_request(user, response);
        }, error);
    }

    void tego_context_send_file_transfer_request(
        tego_context* context,
        tego_user_id_t const*  user,
        char const* filePath,
        size_t filePathLength,
        tego_file_transfer_id_t* out_id,
        tego_file_hash_t** out_fileHash,
        tego_file_size_t* out_fileSize,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            TEGO_THROW_IF_NULL(filePath);
            TEGO_THROW_IF_FALSE(filePathLength > 0);

            auto [id, fileHash, fileSize] =
                context->send_file_transfer_request(
                    user,
                    std::string(filePath, filePathLength));

            if (out_id != nullptr)
            {
                *out_id = id;
            }
            if (out_fileHash != nullptr)
            {
                *out_fileHash = fileHash.release();
            }
            if (out_fileSize != nullptr)
            {
                *out_fileSize = fileSize;
            }

        }, error);
    }

    void tego_context_respond_file_transfer_request(
        tego_context* context,
        tego_user_id_t const* user,
        tego_file_transfer_id_t fileTransfer,
        tego_file_transfer_response_t response,
        char const* destPath,
        size_t destPathLength,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            // dest string must be valid is accept
            TEGO_THROW_IF_TRUE(response == tego_file_transfer_response_accept &&
                (destPath == nullptr || destPathLength == 0))
            // dest string must be null and empty if reject
            TEGO_THROW_IF_TRUE(response == tego_file_transfer_response_reject &&
                (destPath != nullptr || destPathLength > 0))

            context->respond_file_transfer_request(
                user,
                fileTransfer,
                response,
                destPath ? std::string(destPath, destPathLength) : std::string());
        }, error);
    }

    void tego_context_cancel_file_transfer(
        tego_context* context,
        tego_user_id_t const* user,
        tego_file_transfer_id_t id,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            context->cancel_file_transfer_transfer(user, id);
        }, error);
    }

    void tego_context_send_message(
        tego_context_t* context,
        const tego_user_id_t* user,
        const char* message,
        size_t messageLength,
        tego_message_id_t* out_id,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            TEGO_THROW_IF_NULL(user);
            TEGO_THROW_IF_NULL(message);
            TEGO_THROW_IF_FALSE(messageLength > 0);

            auto id = context->send_message(user, std::string(message, messageLength));
            if (out_id != nullptr)
            {
                logger::println("Sent message with id: {}", id);
                *out_id = id;
            }
        }, error);
    }

    void tego_context_forget_user(
        tego_context_t* context,
        const tego_user_id_t* user,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(context);
            TEGO_THROW_IF_FALSE(context->threadId == std::this_thread::get_id());
            context->forget_user(user);
        }, error);
    }
}
