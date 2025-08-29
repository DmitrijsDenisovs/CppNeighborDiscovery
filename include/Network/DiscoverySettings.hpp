#pragma once
#ifndef DISCOVERYSETTINGS_HPP
#define DISCOVERYSETTINGS_HPP

#include <cstdint>

namespace Network {
    class DiscoverySettings {
    public:
        std::uint16_t port;
        unsigned int sendingPeriodS;
        unsigned int neighborActivityPeriodS;
        unsigned int maxBufferSize;
    };
}

#endif