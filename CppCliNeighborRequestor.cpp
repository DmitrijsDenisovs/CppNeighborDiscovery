#include "include/Unix/UnixSocket.hpp"
#include "include/Config/Config.hpp"
#include "include/Utility/Serialization/Deserializer.hpp"
#include "include/Network/NetInterfaces/NetInterface.hpp"
#include "include/Network/NetInterfaces/IPv4Info.hpp"
#include "include/Network/NetInterfaces/IPv6Info.hpp"
#include "include/Network/NetInterfaces/NetInterfaceManager.hpp"

#include <iostream>
#include <vector>
#include <cstdint>
#include <chrono>
#include <ranges>
#include <algorithm>

using Unix::UnixSocket;
using Utility::Serialization::Deserializer;
using Network::NetInterfaces::NetInterface;
using Network::NetInterfaces::IPv4Info;
using Network::NetInterfaces::IPv6Info;
using Network::NetInterfaces::NetInterfaceManager;

int main() {
    auto clientReturn = UnixSocket::clientFactory(Config::UNIX_DOMAIN_SOCKET_PATH);
    if (!clientReturn.isOk()) {
        std::cout << "Failed opening UNIX domain socket on " << Config::UNIX_DOMAIN_SOCKET_PATH << ": " << clientReturn.msg.value() << std::endl;
        return -1;
    }

    auto client = std::move(clientReturn.data.value());

    //request neigbhor list
    auto sendReturn = client.send(Config::UNIX_DOMAIN_REQUEST_COMMAND);
    if (!sendReturn.isOk()) {
        std::cout << "Failed sending on UNIX domain socket on " << Config::UNIX_DOMAIN_SOCKET_PATH << ": " << sendReturn.msg.value() << std::endl;
        return -1;
    }


    std::vector<std::uint8_t> buff{};
    bool received = false;
    auto start = std::chrono::steady_clock::now();
    do {
        std::vector<std::uint8_t> rbuff(Config::SINGLE_MESSAGE_MAX_SIZE_BYTES);
        auto receiveReturn = client.receive(rbuff);
        if (!receiveReturn.isOk()) {
            std::cout << "Failed receiving on UNIX domain socket on " << Config::UNIX_DOMAIN_SOCKET_PATH << ": " << receiveReturn.msg.value() << std::endl;
            return -1;
        }
        if (rbuff.size() > 0) {
            received = true;
            buff = std::move(rbuff);
        }
    } while (!received 
        && std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count() < Config::CLI_REQUEST_WAIT_TIME_SECONDS);

    if (!received) {
        std::cout << "Couldn't receive data from daemon" << std::endl;
        return -1;
    }

    auto desReturn = Deserializer::deserialize<NetInterface>(buff);
    if (!desReturn.isOk()) {
        std::cout << "Failed deserializing: " << desReturn.msg.value() << std::endl;
        return -1;
    }

    std::vector<NetInterface> neighbors = desReturn.data.value();

    auto nifsReturn = NetInterfaceManager::getInterfaces(); 
    if (!nifsReturn.isOk()) {
        std::cout << "Couldn't check local network interfaces:" << nifsReturn.msg.value();
    }

    auto localMacs = nifsReturn.data.value() | std::ranges::views::transform([](const NetInterface& nif){ return nif.mac; });

    std::cout << "Current neighbors: \n";
    int i = 0;
    for (const auto& nif : neighbors) {
        std::string local = std::ranges::find(localMacs, nif.mac) == localMacs.end() ? "" : "LOCAL ";
        std::cout << ++i << ") " << local << nif.mac << "\n";
        std::cout << "\tSame subnet IPv4s: \n";
        for (const auto& ipv4 : nif.ipv4s) {    
            std::cout <<  "\t - " << ipv4.address << " (" << ipv4.netmask  << " netmask)" << "\n";
        }
        std::cout << "\tSame subnet IPv6s: \n";
        for (const auto& ipv6 : nif.ipv6s) {    
            std::cout <<  "\t - " << ipv6.address << " (" << static_cast<int>(ipv6.prefixLength) << " prefix length)" << "\n";
        }
    }
    std::cout << std::endl;

    return 0;
}