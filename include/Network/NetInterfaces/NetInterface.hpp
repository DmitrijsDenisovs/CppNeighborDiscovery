#pragma once
#ifndef NETINTERFACE_HPP
#define NETINTERFACE_HPP

#include "IPv4Info.hpp"
#include "IPv6Info.hpp"
#include "Utility/Serialization/ISerializable.hpp"
#include "Utility/Serialization/IDeserializable.hpp"
#include "Utility/FunctionReturn.hpp"

#include <string>
#include <vector>
#include <cstdint>

using Utility::Serialization::ISerializable;
using Utility::FunctionReturn;

namespace Network::NetInterfaces {
    struct NetInterface : public ISerializable, public IDeserializable<NetInterface> {
        std::string name;
        std::string mac;
        std::vector<IPv4Info> ipv4s;
        std::vector<IPv6Info> ipv6s;

        ~NetInterface() = default;
        void serialize(std::vector<std::uint8_t>& buff) const;
        //used by IDeserializable interface to handle deserialization statically
        static FunctionReturn<NetInterface> deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset);

        bool operator==(const NetInterface& other) const {
            return this->name == other.name
                && this->mac == other.mac
                && this->ipv4s == other.ipv4s
                && this->ipv6s == other.ipv6s;
        }
    };
}

#endif