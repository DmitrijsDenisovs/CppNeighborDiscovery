#pragma once
#ifndef DESERIALIZER_HPP
#define DESERIALIZER_HPP

#include "FunctionReturn.hpp"

#include <cstdint>
#include <vector>
#include <string>
#include <concepts>
#include <type_traits>
#include <cstring>

using Utility::FunctionReturn;
using Utility::ExitCode;

namespace Utility::Serialization {

    //concept that requires type to have a static deserialize method
    template<typename T>
    concept Deserializable = requires(T t, std::vector<std::uint8_t>& buff, std::size_t& offset) {
        { T::deserialize(buff, offset) } -> std::same_as<FunctionReturn<T>>;
    };

    //handles deserialization of primitives, strings, vectors of deserializable classes
    //vector and string deserialization is handled by acquiring length first, compound types' deserialization order handled in concrete classes
    //deserializes from vector of bytes
    //deserialization shoudln't fail unless buffers are mutated outside of this program
    class Deserializer {
    public:
    //functions made inline to fix multiple definition problems
        template<typename T>
            requires (std::is_trivial_v<T>)
        inline static FunctionReturn<T> deserialize(const std::vector<std::uint8_t>& buff, std::size_t& offset);

        inline static FunctionReturn<std::string> deserialize(const std::vector<std::uint8_t>& buff,  std::size_t& offset);

        template<Deserializable T>
        inline static FunctionReturn<std::vector<T>> deserialize(const std::vector<std::uint8_t>& buff, std::size_t& offset);

        template<typename T>
            requires (std::is_trivial_v<T>)
        static FunctionReturn<T> deserialize(const std::vector<std::uint8_t>& buff) {
            std::size_t offset = 0;
            return deserialize<T>(buff, offset);
        }

        static FunctionReturn<std::string> deserialize(const std::vector<std::uint8_t>& buff) {
            std::size_t offset = 0;
            return deserialize(buff, offset);
        }

        template<Deserializable T>
        static FunctionReturn<std::vector<T>> deserialize(const std::vector<std::uint8_t>& buff) {
            std::size_t offset = 0;
            return deserialize<T>(buff, offset);
        }
    };

    template<typename T>
        requires (std::is_trivial_v<T>)
    FunctionReturn<T> Deserializer::deserialize(const std::vector<std::uint8_t>& buff, std::size_t& offset) {
        if (offset + sizeof(T) > buff.size()) {
            return FunctionReturn<T>{ExitCode::Error, "Primitive deserialization failed, overflow"};
        }
        T val;
        std::memcpy(&val, buff.data() + offset, sizeof(T));
        offset += sizeof(T);

        return FunctionReturn<T>{val};
    }

    FunctionReturn<std::string> Deserializer::deserialize(const std::vector<std::uint8_t>& buff,  std::size_t& offset) {
        auto funcReturn = Deserializer::deserialize<std::uint64_t>(buff, offset);
        if (!funcReturn.isOk()) {
            return FunctionReturn<std::string>{"String deserialization failed", funcReturn};
        }
        std::uint64_t length = funcReturn.data.value();

        if (offset + length > buff.size()) {
            // , msg = "String deserialization failed, overflow"
            return FunctionReturn<std::string>{ExitCode::Error, "String deserialization failed, overflow"};
        }
        std::string str(reinterpret_cast<const char*>(buff.data() + offset), length);
        offset += length;

        return  FunctionReturn<std::string>{str};
    }

    template<Deserializable T>
    FunctionReturn<std::vector<T>> Deserializer::deserialize(const std::vector<std::uint8_t>& buff, std::size_t& offset) {
        auto funcReturn = Deserializer::deserialize<std::uint64_t>(buff, offset);
        if (!funcReturn.isOk()) {
            return FunctionReturn<std::vector<T>>{"Vector deserialization failed", funcReturn};
        }
        std::uint64_t length = funcReturn.data.value();

        std::vector<T> vec;
        vec.reserve(length);
        for (uint64_t i = 0; i < length; ++i) {
            auto funcReturnElem = T::deserialize(buff, offset);
            if (!funcReturnElem.isOk()) {
                return FunctionReturn<std::vector<T>>{"Vector deserialization failed", funcReturnElem};
            }
            vec.push_back(funcReturnElem.data.value());
        }

        return FunctionReturn<std::vector<T>>{vec};
    }
}
#endif