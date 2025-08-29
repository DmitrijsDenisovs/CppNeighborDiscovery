#pragma once
#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include <vector>
#include <cstdint>
#include <type_traits>
#include <string>
#include <concepts>
#include <iostream>

namespace Utility::Serialization {

    //concept enforces type to have a serialize method
    template<typename T>
    concept Serializable = requires(T t, std::vector<std::uint8_t>& buff) {
        { t.serialize(buff) } -> std::same_as<void>;
    };

    //handles serialization of primitive types, strings and vectors of serializable types
    //vectors and strings serialize their length first, order serialization of of compound types is defined in concrete types
    //serialization result is vector of bytes, stored in a passed buffer
    class Serializer {
    public:
        //functions made inline to fix multiple definition problems
        template<typename T>
            requires (std::is_trivial_v<T>)
        inline static void serialize(std::vector<std::uint8_t>& buff, const T& val);

        inline static void serialize(std::vector<std::uint8_t>& buff, const std::string& str);

        template<Serializable T>
        inline static void serialize(std::vector<std::uint8_t>& buff, const std::vector<T>& vec);
    };

    template<typename T>
        requires (std::is_trivial_v<T>)
    void Serializer::serialize(std::vector<std::uint8_t>& buff, const T& val) {
        const std::uint8_t* pointer = reinterpret_cast<const std::uint8_t*>(&val);
        buff.insert(buff.end(), pointer, pointer + sizeof(T));
    }

    void Serializer::serialize(std::vector<std::uint8_t>& buff, const std::string& str) {
        std::uint64_t length = str.size();
        Serializer::serialize(buff, length);
        buff.insert(buff.end(), str.begin(), str.end());
    }

    template<Serializable T>
    void Serializer::serialize(std::vector<std::uint8_t>& buff, const std::vector<T>& vec) {
        std::uint64_t length = vec.size();
        Serializer::serialize(buff, length);

        for (const auto& item : vec) {
            item.serialize(buff);
        }
    }


}

#endif