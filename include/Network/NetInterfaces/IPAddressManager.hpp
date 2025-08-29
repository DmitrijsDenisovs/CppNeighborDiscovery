#pragma once
#ifndef IPADDRESSMANAGER_HPP
#define IPADDRESSMANAGER_HPP

#include "IPv4Info.hpp"
#include "IPv6Info.hpp"

namespace Network::NetInterfaces {
    class IPAddressManager {
    public:
        static bool isSameSubnet(const Network::NetInterfaces::IPv4Info&, const Network::NetInterfaces::IPv4Info&);
        static bool isSameSubnet(const Network::NetInterfaces::IPv6Info&, const Network::NetInterfaces::IPv6Info&);
    };
}

#endif