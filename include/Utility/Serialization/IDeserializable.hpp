#pragma once
#ifndef IDESERIALIZABLE_HPP
#define IDESERIALIZABLE_HPP

#include "FunctionReturn.hpp"

#include <vector>
#include <cstdint>
#include <concepts>
#include <type_traits>

using Utility::FunctionReturn;

namespace Utility::Serialization {

    template<typename T>
    concept DeserializableImplemented = requires(T t, std::vector<std::uint8_t>& buff, std::size_t& offset) {
        { T::deserializeImpl(buff, offset) } -> std::same_as<FunctionReturn<T>>;
    };

    //ensures that derived have deserializeImpl method and makes deserialize static
    template<typename TDerived>
    class IDeserializable {
    public:
        static FunctionReturn<TDerived> deserialize(const std::vector<std::uint8_t> buff, std::size_t& offset) {
            return TDerived::deserializeImpl(buff, offset);
        }

        virtual ~IDeserializable() = default;
    };
}

#endif