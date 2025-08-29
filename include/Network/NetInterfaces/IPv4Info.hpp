#pragma once
#ifndef IPV4INFO_HPP
#define IPV4INFO_HPP

#include "Utility/Serialization/ISerializable.hpp"
#include "Utility/Serialization/IDeserializable.hpp"
#include "Utility/FunctionReturn.hpp"

#include <string>
#include <cstdint>
#include <vector>

using Utility::Serialization::ISerializable;
using Utility::Serialization::IDeserializable;
using Utility::FunctionReturn;

namespace Network::NetInterfaces {
    struct IPv4Info : public ISerializable, public IDeserializable<IPv4Info> {
        std::string address;
        std::string netmask;

        IPv4Info(std::string address, std::string netmask) : address{address}, netmask{netmask} {}
        ~IPv4Info() = default;

        void serialize(std::vector<std::uint8_t>& buff) const;
        //used by IDeserializable interface to handle deserialization statically
        static FunctionReturn<IPv4Info> deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset);

        bool operator==(const IPv4Info& other) const {
            return this->address == other.address
                && this->netmask == other.netmask;
        }
    };
}

#endif