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
}