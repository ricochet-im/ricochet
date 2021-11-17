#include "TorControl.h"

namespace shims
{
    TorControl* TorControl::torControl = nullptr;

    TorControl::TorControl(tego_context_t* context_)
    : context(context_)
    { }

    QObject* TorControl::setConfiguration(const QVariantMap &options)
    {
        logger::trace();
        Q_ASSERT(this->m_setConfigurationCommand == nullptr);

        // create command shim yuck
        auto setConfigurationCommand = new TorControlCommand();
        QQmlEngine::setObjectOwnership(setConfigurationCommand, QQmlEngine::CppOwnership);

        this->m_setConfigurationCommand = setConfigurationCommand;

        std::unique_ptr<tego_tor_daemon_config_t> daemonConfig;
        tego_tor_daemon_config_initialize(
            tego::out(daemonConfig),
            tego::throw_on_error());

        //
        // see TorConfigurationPage.xml
        //

        // disable network
        if (auto it = options.find("disableNetwork"); it != options.end())
        {
            const auto disableNetwork = options.value("disableNetwork").toInt();
            Q_ASSERT(disableNetwork == TEGO_FALSE || disableNetwork == TEGO_TRUE);

            tego_tor_daemon_config_set_disable_network(
                daemonConfig.get(),
                static_cast<tego_bool_t>(disableNetwork),
                tego::throw_on_error());
        }

        // proxy
        if (auto it = options.find("proxyType"); it != options.end() && *it != "")
        {
            Q_ASSERT(options.contains("proxyAddress"));
            Q_ASSERT(options.contains("proxyPort"));

            // we always need these params
            const auto proxyAddress = options.value("proxyAddress").toString().toUtf8();
            const auto proxyPort = options.value("proxyPort").toInt();

            // ensure vali proxy type
            const auto& proxyType = *it;
            Q_ASSERT(proxyType == "socks4" || proxyType == "socks5" || proxyType == "https");

            // handle socks4
            if (proxyType == "socks4") {

                Q_ASSERT(proxyPort > 0 && proxyPort < 65536);

                tego_tor_daemon_config_set_proxy_socks4(
                    daemonConfig.get(),
                    proxyAddress.data(),
                    static_cast<size_t>(proxyAddress.size()),
                    static_cast<uint16_t>(proxyPort),
                    tego::throw_on_error());
            }
            // handle socks5 and https
            else if (proxyType == "socks5" || proxyType == "https")
            {
                Q_ASSERT(options.contains("proxyUsername"));
                Q_ASSERT(options.contains("proxyPassword"));

                const auto proxyUsername = options.value("proxyUsername").toString().toUtf8();
                const auto proxyPassword = options.value("proxyPassword").toString().toUtf8();

                if (proxyType == "socks5")
                {
                    tego_tor_daemon_config_set_proxy_socks5(
                        daemonConfig.get(),
                        proxyAddress.data(),
                        static_cast<size_t>(proxyAddress.size()),
                        static_cast<uint16_t>(proxyPort),
                        proxyUsername.data(),
                        static_cast<size_t>(proxyUsername.size()),
                        proxyPassword.data(),
                        static_cast<size_t>(proxyPassword.size()),
                        tego::throw_on_error());
                }
                else if (proxyType == "https")
                {
                    tego_tor_daemon_config_set_proxy_https(
                        daemonConfig.get(),
                        proxyAddress.data(),
                        static_cast<size_t>(proxyAddress.size()),
                        static_cast<uint16_t>(proxyPort),
                        proxyUsername.data(),
                        static_cast<size_t>(proxyUsername.size()),
                        proxyPassword.data(),
                        static_cast<size_t>(proxyPassword.size()),
                        tego::throw_on_error());
                }
            }
        }

        // allowed ports
        if (auto it = options.find("allowedPorts"); it != options.end())
        {
            auto portList = it->toList();

            if (portList.size() > 0)
            {
                std::vector<uint16_t> ports;
                ports.reserve(static_cast<size_t>(portList.size()));

                // convert port list
                for(const auto& v : portList)
                {
                    auto currentPort = v.toInt();
                    Q_ASSERT(currentPort > 0 && currentPort < 65536);
                    ports.push_back(static_cast<uint16_t>(currentPort));
                }

                tego_tor_daemon_config_set_allowed_ports(
                    daemonConfig.get(),
                    ports.data(),
                    ports.size(),
                    tego::throw_on_error());
            }
        }

        // bridges
        if (auto it = options.find("bridges"); it != options.end())
        {
            // get the bridge list
            auto bridgeList = it->toList();

            if (bridgeList.size() > 0)
            {
                // convert the bridges to strings
                std::vector<std::string> bridgeStrings;
                bridgeStrings.reserve(static_cast<size_t>(bridgeList.size()));

                // create a vector of said strings, so that the raw pointers have a lifetime for the rest of this scope
                for(auto& v : bridgeList)
                {
                    auto currentBridge = v.toString();
                    bridgeStrings.push_back(currentBridge.toStdString());
                }
                const size_t bridgeCount = bridgeStrings.size();

                // copy over raw
                auto rawBridges = std::make_unique<char* []>(bridgeCount);
                auto rawBridgeLengths = std::make_unique<size_t[]>(bridgeCount);

                std::fill(rawBridges.get(), rawBridges.get() + bridgeCount, nullptr);
                std::fill(rawBridgeLengths.get(), rawBridgeLengths.get()+ bridgeCount, 0);

                for(size_t i = 0; i < bridgeStrings.size(); ++i)
                {
                    const auto& currentBridgeString = bridgeStrings[i];
                    rawBridges[i] = const_cast<char*>(currentBridgeString.data());
                    rawBridgeLengths[i] = currentBridgeString.size();
                }

                tego_tor_daemon_config_set_bridges(
                    daemonConfig.get(),
                    const_cast<const char**>(rawBridges.get()),
                    rawBridgeLengths.get(),
                    bridgeCount,
                    tego::throw_on_error());
            }
        }

        tego_context_update_tor_daemon_config(
            context,
            daemonConfig.get(),
            tego::throw_on_error());

        return this->m_setConfigurationCommand;
    }

    // for now we just assume we always have ownership,
    // as we have no way in config to setup usage of
    // an existing tor process
    bool TorControl::hasOwnership() const
    {
        logger::trace();
        return true;
    }

    void TorControl::saveConfiguration()
    {
        logger::trace();
        tego_context_save_tor_daemon_config(
            context,
            tego::throw_on_error());
    }

    QString TorControl::torVersion() const
    {
        logger::trace();
        return tego_context_get_tor_version_string(
            context,
            tego::throw_on_error());
    }

    TorControl::Status TorControl::status() const
    {
        tego_tor_control_status_t status;
        tego_context_get_tor_control_status(
            context,
            &status,
            tego::throw_on_error());

        logger::trace();
        return static_cast<TorControl::Status>(status);
    }

    TorControl::TorStatus TorControl::torStatus() const
    {
        tego_tor_network_status_t status;
        tego_context_get_tor_network_status(
            context,
            &status,
            tego::throw_on_error());

        logger::trace();
        switch(status)
        {
            case tego_tor_network_status_unknown:
                return TorControl::TorUnknown;
            case tego_tor_network_status_ready:
                return TorControl::TorReady;
            case tego_tor_network_status_offline:
                return TorControl::TorOffline;
            default:
                return TorControl::TorError;
        }
    }

    QVariantMap TorControl::bootstrapStatus() const
    {
        QVariantMap retval;
        try
        {
            int32_t progress;
            tego_tor_bootstrap_tag_t tag;
            tego_context_get_tor_bootstrap_status(
                context,
                &progress,
                &tag,
                tego::throw_on_error());

            auto tagSummary = tego_tor_bootstrap_tag_to_summary(
                    tag,
                    tego::throw_on_error());

            retval["severity"] = "mock severity";
            retval["progress"] = progress;
            retval["tag"] = (tag == tego_tor_bootstrap_tag_done) ? "done" : "mock_tag";
            retval["summary"] = QString(tagSummary);
        }
        catch(const std::exception& ex)
        {
            logger::println("Exception thrown : {}", ex.what());
        }
        return retval;
    }

    QString TorControl::errorMessage() const
    {
        return m_errorMessage;
    }

    void TorControl::setStatus(Status status)
    {
        auto oldStatus = m_status;
        if (oldStatus == status) return;

        m_status = status;
        emit this->statusChanged(
            static_cast<int>(status),
            static_cast<int>(oldStatus));
    }

    void TorControl::setTorStatus(TorStatus status)
    {
        auto oldStatus = m_torStatus;
        if (oldStatus == status) return;

        m_torStatus = status;
        emit this->torStatusChanged(
            static_cast<int>(status),
            static_cast<int>(oldStatus));
    }

    void TorControl::setErrorMessage(const QString& msg)
    {
        m_errorMessage = msg;
        this->setStatus(TorControl::Error);
    }
}
