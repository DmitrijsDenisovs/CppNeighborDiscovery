#include "NetworkNeighborDiscoverer.hpp"

#include "NetInterfaces/NetInterfaceManager.hpp"
#include "Sockets/IPMulticastSender.hpp"
#include "Sockets/IPMulticastReceiver.hpp"
#include "Sockets/Constants/IPAddresses.hpp"
#include "Utility/Serialization/Deserializer.hpp"
#include "Utility/Serialization/Serializer.hpp"
#include "Network/NetInterfaces/IPAddressManager.hpp"
#include "Network/NetInterfaces/IPv4Info.hpp"
#include "Network/NetInterfaces/IPv6Info.hpp"

#include <net/if.h>
#include <syslog.h>

#include <cstdint>
#include <chrono>
#include <memory>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string>
#include <sstream>
#include <unordered_map>

using Network::NetworkNeighborDiscoverer;
using Network::NetInterfaces::IPv4Info;
using Network::NetInterfaces::IPv6Info;
using Network::NetInterfaces::NetInterfaceManager;
using Network::Sockets::Constants::IPAddresses;
using Network::Sockets::IPMulticastSender;
using Network::Sockets::IPMulticastReceiver;
using Unix::UnixSocket;
using Network::NetInterfaces::NetInterface;
using Network::NetInterfaces::IPAddressManager;
using Utility::Serialization::Deserializer;
using Utility::Serialization::Serializer;


void NetworkNeighborDiscoverer::runIteration() {

    //get system's network interfaces
    std::vector<NetInterface> nifs{};
    auto nifReturn = NetInterfaceManager::getInterfaces();
    
    if (nifReturn.isOk()) {
        nifs = nifReturn.data.value();
    } else {
        if (this->logger != nullptr) {
            logger->error("Couldn't retrieve system's network interfaces :" + nifReturn.msg.value());
        }
    }

    if (nifs.size() > 0) {
        bool canUseIPv6 = std::ranges::any_of(nifs, [](const NetInterface& nif) { return nif.ipv6s.size() > 0; });
        bool canUseIPv4 = std::ranges::any_of(nifs, [](const NetInterface& nif) { return nif.ipv6s.size() > 0; });

        if (prevNifs != nifs) {
            if (this->logger != nullptr) {
                if (this->logger != nullptr) {
                    auto macsVec = nifs | std::views::transform([](const NetInterface& nif){ return nif.mac; });    
                    std::ostringstream macs;
                    for (const auto& mac : macsVec) {
                        macs << mac << " ";
                    }
                    logger->info("Found system's interfaces with MACs: " + macs.str());
                }
            }

            //update senders/receivers
            auto nifsToDisable = this->prevNifs
                | std::views::filter([&](const NetInterface& nif){
                    return std::ranges::find(nifs, nif) == nifs.end();
                });
            
            for (const auto& nif : nifsToDisable) {
                if (this->ipv6receiver != nullptr) {
                    auto disReturn = this->ipv6receiver
                        ->disableMulticastGroup(IPAddresses::IPv6CustomMulticast, ::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't disable IPv6 multicast receival on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
                
                if (this->ipv6sender != nullptr) {
                    auto disReturn = this->ipv6sender
                        ->removeMulticastAddress(IPAddresses::IPv6CustomMulticast, this->settings.port,::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't disable IPv6 multicast sending on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }

                if (this->ipv4receiver != nullptr) {
                    auto disReturn = this->ipv4receiver
                        ->disableMulticastGroup(IPAddresses::IPv4CustomMulticast, ::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't disable IPv4 multicast receival on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
                
                if (this->ipv4sender != nullptr) {
                    auto disReturn = this->ipv4sender
                        ->removeMulticastAddress(IPAddresses::IPv4CustomMulticast, this->settings.port, ::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't disable IPv4 multicast sending on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
            }

            auto nifsToEnable = nifs
                | std::views::filter([&](const NetInterface& nif){
                    return std::ranges::find(this->prevNifs, nif) == this->prevNifs.end();
                });

            for (const auto& nif : nifsToEnable) {
                if (this->ipv6receiver != nullptr) {
                    auto disReturn = this->ipv6receiver
                        ->enableMulticastGroup(IPAddresses::IPv6CustomMulticast, ::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't enable IPv6 multicast receival on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
                
                if (this->ipv6sender != nullptr) {
                    auto disReturn = this->ipv6sender
                        ->addMulticastAddress(IPAddresses::IPv6CustomMulticast, this->settings.port,::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't enable IPv6 multicast sending on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }

                if (this->ipv4receiver != nullptr) {
                    auto disReturn = this->ipv4receiver
                        ->enableMulticastGroup(IPAddresses::IPv4CustomMulticast, ::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't enable IPv4 multicast receival on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
                
                if (this->ipv4sender != nullptr) {
                    auto disReturn = this->ipv4sender
                        ->addMulticastAddress(IPAddresses::IPv4CustomMulticast, this->settings.port,::if_nametoindex(nif.name.c_str()));
                    if (this->logger != nullptr && !disReturn.isOk()) {
                        this->logger->error(std::format("Couldn't enable IPv4 multicast sending on {}", nif.name) + ": " + disReturn.msg.value());
                    }
                }
            }

        }

        //send own interfaces
        {
            std::vector<std::uint8_t> sbuff{};
            Serializer::serialize<NetInterface>(sbuff, nifs);

            if (canUseIPv6 && this->ipv6sender != nullptr) {
                this->ipv6sender->send(sbuff);
            }

            if (canUseIPv4 && this->ipv4sender != nullptr) {
                this->ipv4sender->send(sbuff);
            }
        }

        //receive 
        std::unordered_map<std::string, NetInterface> receivedNifs{};
        bool receivedRequestFromCliClient = false;
        std::unique_ptr<UnixSocket> clientSocket = nullptr;
        {
            auto receiveStart = std::chrono::steady_clock::now();
            unsigned int receiveTimePassedS = 0;
            int receivedBytes = 0;
            do {
                if (canUseIPv6 && this->ipv6sender != nullptr) {
                    std::vector<std::uint8_t> rbuff(this->settings.maxBufferSize);
                    ::sockaddr_in6 sender{};
                    int n = this->ipv6receiver->receive(rbuff, &sender);

                    if (n > 0) {
                        if (this->logger != nullptr) {
                            char addrbuf[INET6_ADDRSTRLEN];
                            ::inet_ntop(AF_INET6, &sender.sin6_addr, addrbuf, sizeof(addrbuf));
                            this->logger->info(std::format("Received {}B from {}", n, addrbuf));
                        }
                        auto desReturn = Deserializer::deserialize<NetInterface>(rbuff);
                        if (desReturn.isOk()) {
                            auto ipv6ReceivedNifs = desReturn.data.value();

                            for (const NetInterface& nif : ipv6ReceivedNifs) {
                                receivedNifs[nif.mac] = nif;
                            }
                        } else {
                            if (this->logger != nullptr) {
                                this->logger->error("Couldn't deserialize data: " + desReturn.msg.value());
                            }
                        }

                        receivedBytes += n;
                    }
                }

                if (canUseIPv4 && this->ipv4sender != nullptr) {
                    std::vector<std::uint8_t> rbuff(this->settings.maxBufferSize);
                    ::sockaddr_in sender{};
                    int n = this->ipv4receiver->receive(rbuff, &sender);

                    if (n > 0) {
                        if (this->logger != nullptr) {
                            char addrbuf[INET_ADDRSTRLEN];
                            ::inet_ntop(AF_INET, &sender.sin_addr, addrbuf, sizeof(addrbuf));
                            this->logger->info(std::format("Received {}B from {}", n, addrbuf));
                        }
                        receivedBytes += n;
                        auto desReturn = Deserializer::deserialize<NetInterface>(rbuff);
                        if (desReturn.isOk()) {
                            auto ipv4ReceivedNifs = desReturn.data.value();

                            for (const NetInterface& nif : ipv4ReceivedNifs) {
                                receivedNifs[nif.mac] = nif;
                            }
                        } else {
                            if (this->logger != nullptr) {
                                this->logger->error("Couldn't deserialize data: " + desReturn.msg.value());
                            }
                        }
                    }
                }

                //check if someone requested neigbhor list using cli
                //isOk returns true if client connected
                if (this->unixDomainServer != nullptr) {
                    auto clientSocketReturn = this->unixDomainServer->acceptClient();
                    if (clientSocketReturn.isOk()) {
                        if (this->logger != nullptr) {
                            this->logger->info("Established client connection on UNIX domain socket");
                        }

                        clientSocket = std::make_unique<UnixSocket>(std::move(clientSocketReturn.data.value()));

                        auto recReturn = clientSocket->receiveString(sizeof(this->localSettings.requestString));
                        if (!recReturn.isOk()) {
                            if (this->logger != nullptr) {
                                this->logger->error("Couldn't receive from client over UNIX domain: " + recReturn.msg.value());
                            }
                        } else {
                            if (recReturn.data.value() == this->localSettings.requestString) {
                                if (this->logger != nullptr) {
                                    this->logger->info("Received neigbhor list request from CLI program");
                                }
                                receivedRequestFromCliClient = true;
                            }
                        }
                    }
                }

                receiveTimePassedS = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - receiveStart).count();

            } while(receivedBytes > 0 
                && receiveTimePassedS < this->settings.sendingPeriodS
                && !receivedRequestFromCliClient);
        }

        //handle received network interfaces
        {
            for (const auto& received : (receivedNifs | std::views::values)) {
                std::vector<IPv4Info> matchedIPv4;
                for (const auto& rIPv4 : received.ipv4s) {
                    if (std::ranges::any_of(nifs, [&](const auto& local){
                        return std::ranges::any_of(local.ipv4s, [&](const auto& lIPv4){
                            return NetInterfaces::IPAddressManager::isSameSubnet(lIPv4, rIPv4);
                        });
                    })) {
                        matchedIPv4.push_back(rIPv4);
                    }
                }

                // Filter IPv6 addresses that match any local interface
                std::vector<IPv6Info> matchedIPv6;
                for (const auto& rIPv6 : received.ipv6s) {
                    if (std::ranges::any_of(nifs, [&](const auto& local){
                        return std::ranges::any_of(local.ipv6s, [&](const auto& lIPv6){
                            return NetInterfaces::IPAddressManager::isSameSubnet(lIPv6, rIPv6);
                        });
                    })) {
                        matchedIPv6.push_back(rIPv6);
                    }
                }

                if (!matchedIPv4.empty() || !matchedIPv6.empty()) {
                    NetInterface filteredNif = received;
                    filteredNif.ipv4s = std::move(matchedIPv4);
                    filteredNif.ipv6s = std::move(matchedIPv6);
                    //add/update to timedindexedset
                    this->neighbors.update(filteredNif.mac, filteredNif);
                }
            }

            this->neighbors.remove(std::chrono::seconds(this->settings.neighborActivityPeriodS));
        }

        //send neighbors to CLI client requestor
        if (receivedRequestFromCliClient && this->unixDomainServer != nullptr && clientSocket != nullptr) {
            std::vector<std::uint8_t> clientSBuff;
            Serializer::serialize<NetInterface>(clientSBuff, this->neighbors.data());
            auto sendReturn = clientSocket->send(clientSBuff);
            if (!sendReturn.isOk() && this->logger != nullptr) {
                this->logger->error("Failed to send neighbor list to client: " + sendReturn.msg.value());
            } else if (sendReturn.isOk()) {
                this->logger->info("Sent neighbor list to CLI client");
            }
        }

    }
    this->prevNifs = std::move(nifs);
}

void NetworkNeighborDiscoverer::setupIPv6Sockets() {
    //sender block
    {
        auto funcReturn = IPMulticastSender<::sockaddr_in6>::factory();
        if (funcReturn.isOk()) {
            this->ipv6sender = std::make_unique<IPMulticastSender<::sockaddr_in6>>(
                    std::move(funcReturn.data.value())
                );
        } else {
            if (this->logger != nullptr) {
                this->logger->error("Failed creating IPv6 sender socket: " + funcReturn.msg.value());
            }
            this->ipv6sender = nullptr;
        }
    }

    //receiver block
    {
        auto funcReturn = IPMulticastReceiver<::sockaddr_in6>::factory(this->settings.port);
        if (funcReturn.isOk()) {
            this->ipv6receiver = std::make_unique<IPMulticastReceiver<::sockaddr_in6>>(
                    std::move(funcReturn.data.value())
                );
        } else {
            if (this->logger != nullptr) {
                this->logger->error("Failed creating IPv6 receiver socket: " + funcReturn.msg.value());
            }
            this->ipv6receiver = nullptr;
        }
    }
}

void NetworkNeighborDiscoverer::setupIPv4Sockets() {
    //sender block
    {
        auto funcReturn = IPMulticastSender<::sockaddr_in>::factory();
        if (funcReturn.isOk()) {
            this->ipv4sender = std::make_unique<IPMulticastSender<::sockaddr_in>>(
                    std::move(funcReturn.data.value())
                );
        } else {
            if (this->logger != nullptr) {
                this->logger->error("Failed creating IPv4 sender socket: " + funcReturn.msg.value());
            }
            this->ipv4sender = nullptr;
        }
    }

    //receiver block
    {
        auto funcReturn = IPMulticastReceiver<::sockaddr_in>::factory(this->settings.port);
        if (funcReturn.isOk()) {
            this->ipv4receiver = std::make_unique<IPMulticastReceiver<::sockaddr_in>>(
                    std::move(funcReturn.data.value())
                );
        } else {
            if (this->logger != nullptr) {
                this->logger->error("Failed creating IPv4 receiver socket: " + funcReturn.msg.value());
            }
            this->ipv4receiver = nullptr;
        }
    }
}

void NetworkNeighborDiscoverer::setupUnixDomainSockets() {
    auto funcReturn = UnixSocket::serverFactory(this->localSettings.socketPath);

    if (funcReturn.isOk()) {
        this->unixDomainServer = std::make_unique<UnixSocket>(
                std::move(funcReturn.data.value())
            );
    } else {
        if (this->logger != nullptr) {
            this->logger->error("Couldn't open Unix domain socket: " + funcReturn.msg.value());
        }
        this->unixDomainServer = nullptr;
    }
}