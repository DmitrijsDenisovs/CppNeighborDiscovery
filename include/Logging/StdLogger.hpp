#pragma once
#ifndef STDLOGGER_HPP
#define STDLOGGER_HPP

#include "ILogger.hpp"

#include <string>
#include <iostream>
#include <chrono>
#include <format>
#include <memory>

namespace Logging {
    //singleton class to hanlde logging to std::cout
    class StdLogger : public ILogger {
    private:
        StdLogger() = default;

    public:
        ~StdLogger() = default;

        inline void info(const std::string& message) {
            auto time = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}; 
            std::cout <<  std::format("[{:%F %T}] | INFO: ", time)  << message << std::endl;
        }

        inline void error(const std::string& message) {
            auto time = std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}; 
            std::cout <<  std::format("[{:%F %T}] | ERROR: ", time)  << message << std::endl;
        }

        static std::shared_ptr<StdLogger> getInstance() {
            static std::shared_ptr<StdLogger> instance{new StdLogger()};
            return instance;
        }
    };
}

#endif