#pragma once
#ifndef FUNCTIONRETURN_HPP
#define FUNCTIONRETURN_HPP

#include <optional>
#include <string>
#include <type_traits>

namespace Utility {
    enum class ExitCode {
        Ok = 0,
        Error= 1
    };

    //used as output of functions that's failure can be handled
    template<typename T = void>
    struct FunctionReturn {
        ExitCode code;
        std::optional<T> data;
        std::optional<std::string> msg;

        FunctionReturn(ExitCode code) : code{code} {}
        FunctionReturn(ExitCode code, const std::string& msg) : code{code}, msg{msg} {}

        //template fixes collisions if T = std::string
        template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, std::string>>>
        FunctionReturn(ExitCode code, const T& data) : code{code}, data{data} {}

        FunctionReturn(ExitCode code, T&& data) : code{code}, data{std::move(data)} {}

        FunctionReturn(const T& data) : code{ExitCode::Ok}, data{data} {}

        FunctionReturn(T&& data) : code{ExitCode::Ok}, data{std::move(data)} {}

        ~FunctionReturn() = default;

        //template needed to chain classes of different tempaltes, data is not transfered because chaining made for error message propagation up the call stack
        template<typename U>
        FunctionReturn(const std::string& msg, const FunctionReturn<U>& other) 
        : code{other.code}, msg{msg + ": " + other.msg.value_or("")} {}

        inline bool isOk() const { return this->code == ExitCode::Ok && this->data.has_value(); }
    };

    template<>
    struct FunctionReturn<void> {
        ExitCode code;
        std::optional<std::string> msg;

        FunctionReturn() : code{ExitCode::Ok} {}
        FunctionReturn(const std::string& msg) : code{ExitCode::Error}, msg{msg} {}
        FunctionReturn(ExitCode code) : code{code} {}
        FunctionReturn(ExitCode code, const std::string& msg) : code{code}, msg{msg} {}
        ~FunctionReturn() = default;

        inline bool isOk() const { return this->code == ExitCode::Ok; }
    };
}

#endif