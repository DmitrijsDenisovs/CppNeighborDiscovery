#include "IPAddressManager.hpp"

#include "IPv4Info.hpp"
#include "IPv6Info.hpp"

#include <arpa/inet.h>
#include <cstring>  
#include <cstdint>
#include <climits>

using Network::NetInterfaces::IPAddressManager;
using Network::NetInterfaces::IPv4Info;
using Network::NetInterfaces::IPv6Info;

bool IPAddressManager::isSameSubnet(const IPv4Info& ip1, const IPv4Info& ip2) {
    ::in_addr addr1{}, addr2{}, mask{};
    
    if (ip1.netmask != ip2.netmask) {
        return false;
    }

    if (::inet_pton(AF_INET, ip1.address.c_str(), &addr1) != 1) {
        return false;
    }
    if (::inet_pton(AF_INET, ip2.address.c_str(), &addr2) != 1) {
        return false;
    }
    if (::inet_pton(AF_INET, ip1.netmask.c_str(), &mask) != 1) {
        return false;
    }

    std::uint32_t subnet1bin = ::ntohl(addr1.s_addr) & ::ntohl(mask.s_addr);
    std::uint32_t subnet2bin = ::ntohl(addr2.s_addr) & ::ntohl(mask.s_addr);

    return subnet1bin == subnet2bin;
}

bool IPAddressManager::isSameSubnet(const IPv6Info& ip1, const IPv6Info& ip2) {
    if (ip1.prefixLength != ip2.prefixLength) {
        return false;
    }

    ::in6_addr addr1{}, addr2{};

    if (::inet_pton(AF_INET6, ip1.address.c_str(), &addr1) != 1) {
        return false;
    }
    if (::inet_pton(AF_INET6, ip2.address.c_str(), &addr2) != 1) {
        return false;
    }

    std::uint8_t fullBytes = ip1.prefixLength / CHAR_BIT;
    std::uint8_t remainingBits = ip1.prefixLength & CHAR_BIT;
    if (std::memcmp(addr1.s6_addr, addr2.s6_addr, fullBytes) != 0) {
        return false;
    }

    if (remainingBits > 0) {
        std::uint8_t mask = static_cast<uint8_t>(0xFF << (CHAR_BIT - remainingBits));
        if ( (addr1.s6_addr[fullBytes] & mask) != (addr2.s6_addr[fullBytes] & mask) ) {
            return false;
        }
    }

    return true;
}