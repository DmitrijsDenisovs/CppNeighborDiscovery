#pragma once
#ifndef IPV6INFO_HPP
#define IPV6INFO_HPP

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
    struct IPv6Info : public ISerializable, public IDeserializable<IPv6Info> {
        std::string address;
        std::uint8_t prefixLength;

        IPv6Info(std::string address, std::uint8_t prefixLength) : address{address}, prefixLength{prefixLength} {}
        ~IPv6Info() = default;

        void serialize(std::vector<std::uint8_t>& buff) const;
        //used by IDeserializable interface to handle deserialization statically
        static FunctionReturn<IPv6Info> deserializeImpl(const std::vector<std::uint8_t>& buff, std::size_t& offset);

        bool operator==(const IPv6Info& other) const {
            return this->address == other.address
                && this->prefixLength == other.prefixLength;
        }
    };
}

#endif