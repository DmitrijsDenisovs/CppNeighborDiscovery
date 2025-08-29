#pragma once
#ifndef ISERIALIZABLE_HPP
#define ISERIALIZABLE_HPP

#include <vector>
#include <cstdint>

namespace Utility::Serialization {
    class ISerializable {
    public:
        virtual void serialize(std::vector<std::uint8_t>& buff) const = 0;
        virtual ~ISerializable() = default;
    };
}

#endif