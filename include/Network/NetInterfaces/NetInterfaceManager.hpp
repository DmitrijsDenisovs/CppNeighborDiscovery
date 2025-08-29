#pragma once
#ifndef NETINTERFACEMANAGER_HPP
#define NETINTERFACEMANAGER_HPP

#include "Utility/FunctionReturn.hpp"
#include "NetInterface.hpp"

#include <vector>

using Utility::FunctionReturn;

namespace Network::NetInterfaces {
    //handles gathering of system's available real network interfaces, gets interface name, mac and available IPv4 and IPv6 addresses
    class NetInterfaceManager {
    public:
        static FunctionReturn<std::vector<NetInterface>> getInterfaces();
    };
}

#endif