#include "error.hpp"
#include "tor.hpp"

extern "C"
{
    //
    // Tor Launch Configuration
    //

    void tego_tor_launch_config_initialize(
        tego_tor_launch_config_t** out_launchConfig,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(out_launchConfig);
            TEGO_THROW_IF_FALSE(*out_launchConfig == nullptr);

            auto launchConfig = std::make_unique<tego_tor_launch_config>();
            *out_launchConfig = launchConfig.release();
        }, error);
    }

    void tego_tor_launch_config_set_data_directory(
        tego_tor_launch_config_t* launchConfig,
        const char* dataDirectory,
        size_t dataDirectoryLength,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(launchConfig);
            TEGO_THROW_IF_NULL(dataDirectory);

            logger::println("tor data dir : {}", dataDirectory);

            launchConfig->dataDirectory.assign(dataDirectory, dataDirectoryLength);
        }, error);
    }

    //
    // Tor Daemon Configuration
    //

    void tego_tor_daemon_config_initialize(
        tego_tor_daemon_config_t** out_daemonConfig,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(out_daemonConfig);
            TEGO_THROW_IF_FALSE(*out_daemonConfig == nullptr);

            auto daemonConfig = std::make_unique<tego_tor_daemon_config>();
            *out_daemonConfig = daemonConfig.release();
        }, error);
    }

    void tego_tor_daemon_config_set_disable_network(
        tego_tor_daemon_config_t* config,
        tego_bool_t disableNetwork,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            TEGO_THROW_IF_FALSE(disableNetwork == TEGO_TRUE || disableNetwork == TEGO_FALSE);

            config->disableNetwork = (disableNetwork == TEGO_TRUE);
        }, error);
    }

    void tego_tor_daemon_config_set_proxy_socks4(
        tego_tor_daemon_config_t* config,
        const char* address,
        size_t addressLength,
        uint16_t port,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            TEGO_THROW_IF_NULL(address);
            TEGO_THROW_IF_FALSE(addressLength > 0);
            TEGO_THROW_IF_FALSE(port > 0);

            config->proxy.type = tego_proxy_type_socks4;
            config->proxy.address.assign(address, addressLength);
            config->proxy.port = port;
        }, error);
    }

    void tego_tor_daemon_config_set_proxy_socks5(
        tego_tor_daemon_config_t* config,
        const char* address,
        size_t addressLength,
        uint16_t port,
        const char* username,
        size_t usernameLength,
        const char* password,
        size_t passwordLength,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            TEGO_THROW_IF_NULL(address);
            TEGO_THROW_IF_FALSE(addressLength > 0);
            TEGO_THROW_IF_FALSE(port > 0);
            // if either string is null, their length must also be 0
            TEGO_THROW_IF_FALSE((username == nullptr && usernameLength == 0) || username != nullptr);
            TEGO_THROW_IF_FALSE((password == nullptr && passwordLength == 0) || password != nullptr);

            config->proxy.type = tego_proxy_type_socks5;
            config->proxy.address.assign(address, addressLength);
            config->proxy.port = port;
            if (username != nullptr)
            {
                config->proxy.username.assign(username, usernameLength);
            }
            if (password != nullptr)
            {
                config->proxy.password.assign(password, passwordLength);
            }
        }, error);
    }

    void tego_tor_daemon_config_set_proxy_https(
        tego_tor_daemon_config_t* config,
        const char* address,
        size_t addressLength,
        uint16_t port,
        const char* username,
        size_t usernameLength,
        const char* password,
        size_t passwordLength,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            TEGO_THROW_IF_NULL(address);
            TEGO_THROW_IF_FALSE(addressLength > 0);
            TEGO_THROW_IF_FALSE(port > 0);
            // if either string is null, their length must also be 0
            TEGO_THROW_IF_FALSE((username == nullptr && usernameLength == 0) || username != nullptr);
            TEGO_THROW_IF_FALSE((password == nullptr && passwordLength == 0) || password != nullptr);

            config->proxy.type = tego_proxy_type_https;
            config->proxy.address.assign(address, addressLength);
            config->proxy.port = port;
            if (username != nullptr)
            {
                config->proxy.username.assign(username, usernameLength);
            }
            if (password != nullptr)
            {
                config->proxy.password.assign(password, passwordLength);
            }
        }, error);
    }

     void tego_tor_daemon_config_set_allowed_ports(
        tego_tor_daemon_config_t* config,
        const uint16_t* ports,
        size_t portsCount,
        tego_error_t** error)
     {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            TEGO_THROW_IF_NULL(ports);
            TEGO_THROW_IF_FALSE(portsCount > 0);

            // ensure we only put each port in once
            std::set<uint16_t> portsSet;
            portsSet.insert(ports[0]);
            for(size_t i = 1; i < portsCount; ++i)
            {
                const auto currentPort = ports[i];
                auto it = portsSet.find(currentPort);
                TEGO_THROW_IF_FALSE(it == portsSet.end());

                portsSet.insert(currentPort);
            }

            auto& allowedPorts = config->allowedPorts;
            allowedPorts.clear();
            allowedPorts.reserve(portsCount);

            for(auto currentPort : portsSet)
            {
                allowedPorts.push_back(currentPort);
            }
        }, error);
     }

    void tego_tor_daemon_config_set_bridges(
        tego_tor_daemon_config_t* config,
        const char** bridges,
        size_t* bridgeLengths,
        size_t bridgeCount,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(config);
            // ensure we have valid pointers or we are setting 0 bridges
            TEGO_THROW_IF_FALSE((bridges != nullptr && bridgeLengths != nullptr) || bridgeCount == 0);

            // ensure each bridge string is valid
            for(size_t i = 0; i < bridgeCount; ++i)
            {
                TEGO_THROW_IF_NULL(bridges[i]);
                TEGO_THROW_IF_FALSE(bridgeLengths[i] > 0);
            }

            // clear out existing bridge strings and append our new ones
            auto& confBridges = config->bridges;
            confBridges.clear();

            // copy bridge strings over
            confBridges.reserve(bridgeCount);
            for(size_t i = 0; i < bridgeCount; ++i)
            {
                confBridges.emplace_back(bridges[i], bridgeLengths[i]);
            }
        }, error);
    }
}
