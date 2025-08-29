#pragma once
#ifndef LOGGABLEFROM_HPP
#define LOGGABLEFROM_HPP

#include "ILogger.hpp"

#include <memory>
#include <string>

using Logging::ILogger;

namespace Logging {
    class LoggableFrom {
    protected:
        std::shared_ptr<ILogger> logger;

        void info(const std::string& message) {
            logger->info(message);
        }

        void error(const std::string& message) {
            logger->error(message);
        }

        LoggableFrom(std::shared_ptr<ILogger> logger) : logger{std::move(logger)} {};

    public:
        virtual ~LoggableFrom() = default;
    };
}

#endif