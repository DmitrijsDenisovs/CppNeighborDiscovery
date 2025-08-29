#pragma once
#ifndef UNIXDOMAINSETTINGS_HPP
#define UNIXDOMAINSETTINGS_HPP

#include <string>

namespace Unix {
    //used for unix domain sockets configuration
    class UnixDomainSettings {
    public:
        std::string requestString;
        unsigned int maxBufferSize;
        std::string socketPath;
    };
}

#endif