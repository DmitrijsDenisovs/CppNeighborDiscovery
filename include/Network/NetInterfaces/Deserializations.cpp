#include "NetInterface.hpp"
#include "IPv6Info.hpp"
#include "IPv4Info.hpp"
#include "Utility/Serialization/Deserializer.hpp"
#include "Utility/FunctionReturn.hpp"

#include <vector>
#include <cstdint>

using Network::NetInterfaces::NetInterface;
using Utility::Serialization::Deserializer;
using Network::NetInterfaces::IPv4Info;
using Network::NetInterfaces::IPv6Info;
using Utility::FunctionReturn;

FunctionReturn<NetInterface> NetInterface::deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset) {
    NetInterface nif;

    auto funcReturn = Deserializer::deserialize(buff, offset);
    if (!funcReturn.isOk()) {
        return FunctionReturn<NetInterface>{"Couldn't deserialize network interface name", funcReturn};
    }
    nif.name = funcReturn.data.value();

    auto funcReturn1 = Deserializer::deserialize(buff, offset);
    if (!funcReturn1.isOk()) {
        return FunctionReturn<NetInterface>{"Couldn't deserialize network interface MAC", funcReturn1};
    }
    nif.mac = funcReturn1.data.value();

    auto funcReturn2 = Deserializer::deserialize<IPv4Info>(buff, offset);
    if (!funcReturn2.isOk()) {
        return FunctionReturn<NetInterface>{"Couldn't deserialize IPv4s", funcReturn2};
    }
    nif.ipv4s = funcReturn2.data.value();

    auto funcReturn3 = Deserializer::deserialize<IPv6Info>(buff, offset);
    if (!funcReturn3.isOk()) {
        return FunctionReturn<NetInterface>{"Couldn't deserialize IPv6s", funcReturn3};
    }
    nif.ipv6s = funcReturn3.data.value();

    return FunctionReturn<NetInterface>{ExitCode::Ok, nif};
}

FunctionReturn<IPv4Info> IPv4Info::deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset) {
    auto funcReturn = Deserializer::deserialize(buff, offset);
    if (!funcReturn.isOk()) {
        return FunctionReturn<IPv4Info>{"Couldn't deserialize IPv4 address", funcReturn};
    }
    std::string addr = funcReturn.data.value();

    auto funcReturn1 = Deserializer::deserialize(buff, offset);
    if (!funcReturn1.isOk()) {
        return FunctionReturn<IPv4Info>{"Couldn't deserialize IPv4 netmask", funcReturn1};
    }
    std::string netmask = funcReturn1.data.value();
    return FunctionReturn<IPv4Info>{IPv4Info{addr, netmask}};
}

FunctionReturn<IPv6Info> IPv6Info::deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset) {
    auto funcReturn = Deserializer::deserialize(buff, offset);
    if (!funcReturn.isOk()) {
        return FunctionReturn<IPv6Info>{"Couldn't deserialize IPv6 address", funcReturn};
    }
    std::string addr = funcReturn.data.value();

    auto funcReturn1 = Deserializer::deserialize<std::uint8_t>(buff, offset);
    if (!funcReturn1.isOk()) {
        return FunctionReturn<IPv6Info>{"Couldn't deserialize IPv6 prefix", funcReturn1};
    }
    std::uint8_t prefixLength = funcReturn1.data.value();

    return FunctionReturn<IPv6Info>{IPv6Info{addr, prefixLength}};
}