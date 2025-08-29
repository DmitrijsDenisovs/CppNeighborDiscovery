#include "NetInterface.hpp"
#include "IPv6Info.hpp"
#include "IPv4Info.hpp"
#include "Utility/Serialization/Serializer.hpp"

#include <vector>
#include <cstdint>

using Network::NetInterfaces::NetInterface;
using Utility::Serialization::Serializer;
using Network::NetInterfaces::IPv4Info;
using Network::NetInterfaces::IPv6Info;

void NetInterface::serialize(std::vector<std::uint8_t>& buff) const {
    Serializer::serialize(buff, this->name);
    Serializer::serialize(buff, this->mac);
    Serializer::serialize(buff, this->ipv4s);
    Serializer::serialize(buff, this->ipv6s);
}

void IPv4Info::serialize(std::vector<std::uint8_t>& buff) const {
    Serializer::serialize(buff, this->address);
    Serializer::serialize(buff, this->netmask);
}

void IPv6Info::serialize(std::vector<std::uint8_t>& buff) const {
    Serializer::serialize(buff, this->address);
    Serializer::serialize(buff, this->prefixLength);
}