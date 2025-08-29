#pragma once
#ifndef IPADDRESSES_HPP
#define IPADDRESSES_HPP

#include "Config/Config.hpp"

#include <string>

namespace Network::Sockets::Constants {
    class IPAddresses {
    public:
        inline static const std::string IPv6AllHosts = "ff02::1";
        inline static const std::string IPv4AllHosts = "224.0.0.1";
        inline static const std::string IPv6CustomMulticast = Config::MULTICAST_IPV6;
        inline static const std::string IPv4CustomMulticast = Config::MULTICAST_IPV4;
    };
}

#endif