#pragma once
#ifndef ILOGGER_HPP
#define ILOGGER_HPP

#include <string>

namespace Logging {
    class ILogger {
    public:
        virtual ~ILogger() = default;
        virtual void info(const std::string&) = 0;
        virtual void error(const std::string&) = 0;
    };
}

#endif