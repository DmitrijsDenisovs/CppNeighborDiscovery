#include "NetInterfaceManager.hpp"
#include "Utility/FunctionReturn.hpp"
#include "Logging/SysLogger.hpp"
#include "NetInterface.hpp"

#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>

#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <cstdint>

using Utility::FunctionReturn;
using Utility::ExitCode;
using Network::NetInterfaces::NetInterface;
using Network::NetInterfaces::NetInterfaceManager;

FunctionReturn<std::vector<NetInterface>> NetInterfaceManager::getInterfaces() {
    ::ifaddrs* ifaddr = nullptr;

    if (::getifaddrs(&ifaddr) == -1) {
        return FunctionReturn<std::vector<NetInterface>>{ExitCode::Error, std::vector<NetInterface>()};
    }

    //free structures at the end of method
    std::unique_ptr<::ifaddrs, decltype(&::freeifaddrs)> ifaddr_ptr(ifaddr, ::freeifaddrs);

    std::unordered_map<std::string, NetInterface> interfaces;

    for (auto ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) {
            continue;
        }

        //exclude loopback and unavailable interfaces
        if (ifa->ifa_flags & IFF_LOOPBACK || !(ifa->ifa_flags & IFF_UP)) {
            continue;
        }

        auto& iface = interfaces[ifa->ifa_name];
        iface.name = ifa->ifa_name;

        int family = ifa->ifa_addr->sa_family;

        char addr[INET6_ADDRSTRLEN] = {0};
        char mask[INET6_ADDRSTRLEN] = {0};

        //IPv4
        if (family == AF_INET) {
            auto* sa = reinterpret_cast<::sockaddr_in*>(ifa->ifa_addr);
            auto* nm = reinterpret_cast<::sockaddr_in*>(ifa->ifa_netmask);
            if (::inet_ntop(AF_INET, &sa->sin_addr, addr, sizeof(addr)) &&
                ::inet_ntop(AF_INET, &nm->sin_addr, mask, sizeof(mask))) {
                iface.ipv4s.push_back(IPv4Info{std::string(addr), std::string(mask)});
            }
        }
        //IPv6
        else if (family == AF_INET6) {
            auto* sa = reinterpret_cast<::sockaddr_in6*>(ifa->ifa_addr);
            auto* netmask = reinterpret_cast<::sockaddr_in6*>(ifa->ifa_netmask);

            if (::inet_ntop(AF_INET6, &sa->sin6_addr, addr, sizeof(addr))) {
                std::uint8_t prefixLength = 0;

                if (netmask) {
                    for (size_t i = 0; i < sizeof(::in6_addr); ++i) {
                        uint8_t byte = netmask->sin6_addr.s6_addr[i];
                        for (int b = 7; b >= 0; --b) {
                            if (byte & (1 << b)) {
                                ++prefixLength;
                            } 
                            else {
                                break;
                            }
                        }
                    }
                }

                iface.ipv6s.push_back(IPv6Info{std::string(addr), static_cast<std::uint8_t>(prefixLength)});
            }
        //MAC
        } else if (family == AF_PACKET) {
            auto* sa = reinterpret_cast<::sockaddr_ll*>(ifa->ifa_addr);
            std::ostringstream mac;
            mac << std::hex << std::setfill('0');

            for (int i = 0; i < sa->sll_halen; ++i) {
                mac << std::setw(2) << static_cast<int>(sa->sll_addr[i]);
                if (i < sa->sll_halen - 1) {
                    mac << ":";
                }
            }

            iface.mac = mac.str();
        }
    }

    std::vector<NetInterface> result;
    result.reserve(interfaces.size());

    for (auto &val : interfaces) {
        result.push_back(std::move(val.second));
    }

    return FunctionReturn<std::vector<NetInterface>>{ExitCode::Ok, result};
}