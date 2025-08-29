#pragma once
#ifndef SYSLOGGER_HPP
#define SYSLOGGER_HPP

#include "ILogger.hpp"

#include <string>
#include <syslog.h>
#include <memory>

namespace Logging {
    //singleton class to hanlde logging to syslog
    class SysLogger : public ILogger {
    private:
        SysLogger() {
            ::openlog("", LOG_PID | LOG_CONS, LOG_DAEMON);
        }
    public:
        ~SysLogger() {
            ::closelog();
        }

        inline void info(const std::string& message) {
            ::syslog(LOG_INFO, "%s", message.c_str());
        }

        inline void error(const std::string& message) {
            ::syslog(LOG_ERR, "%s", message.c_str());
        }

        static std::shared_ptr<SysLogger> getInstance() {
            static std::shared_ptr<SysLogger> instance{new SysLogger()};
            return instance;
        }
    };
}
#endif