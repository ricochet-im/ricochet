#pragma once

//
// Tego Tor Configuration
//

struct tego_tor_launch_config
{
    std::string dataDirectory;
};

typedef enum
{
    tego_proxy_type_none = 0,
    tego_proxy_type_socks4,
    tego_proxy_type_socks5,
    tego_proxy_type_https,
} tego_proxy_type_t;

struct tego_tor_daemon_config
{
    std::optional<bool> disableNetwork;
    struct
    {
        tego_proxy_type_t type = tego_proxy_type_none;
        std::string address;
        uint16_t port;
        std::string username;
        std::string password;
    } proxy;
    std::vector<uint16_t> allowedPorts;
    std::vector<std::string> bridges;
};