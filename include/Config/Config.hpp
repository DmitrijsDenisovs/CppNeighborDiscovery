#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>

namespace Config {
    static constexpr std::uint16_t PORT = 5320u;
    static constexpr char MULTICAST_IPV4[] = "239.1.1.1"; //239.0.0.0 subnet
    static constexpr char MULTICAST_IPV6[] = "ff02::100"; //ff02:/16 prefix
    static constexpr unsigned int SENDING_PERIOD_SECONDS = 30u;
    static constexpr unsigned int NEIGHBOR_ACTIVITY_PERIOD_SECONDS = 30u;
    static constexpr unsigned int SINGLE_MESSAGE_MAX_SIZE_BYTES = 12800u;

    static constexpr char UNIX_DOMAIN_REQUEST_COMMAND[] = "request";
    static constexpr char UNIX_DOMAIN_SOCKET_PATH[] = "/tmp/cppneigbhordiscovery.sock";
    
    static constexpr unsigned int CLI_REQUEST_WAIT_TIME_SECONDS = 10u;

    static constexpr char STD_REDIRECT_PATH[] = "/tmp/cppneighbordiscovery.log"; //make "" empty to redirect to std::cout
    static unsigned int ITERATION_PERIOD_SECONDS = 20u;
}

#endif