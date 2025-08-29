#pragma once
#ifndef IPMULTICASTSENDER_HPP
#define IPMULTICASTSENDER_HPP

#include "Utility/FunctionReturn.hpp"
#include "Logging/LoggableFrom.hpp"
#include "Logging/ILogger.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <net/if.h> 

#include <type_traits>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <memory>
#include <format>
#include <cstring>

using Utility::FunctionReturn;
using Utility::ExitCode;

namespace Network::Sockets {
    //templated socket handler for multicast message sending (::sockaddr_in for IPv4, ::sockaddr_in fo IPv6)
    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    class IPMulticastSender {
    private:
        int sockFd;
        int family;
        //wraps sockaddr and interface
        struct Target {
            T addr{};
            unsigned int ifindex{};
        };
        std::vector<Target> targets;

        IPMulticastSender();

    public:
        ~IPMulticastSender();

        FunctionReturn<> addMulticastAddress(const std::string& multicast_ip, std::uint16_t port, const unsigned int& ifindex = 0);
        FunctionReturn<> removeMulticastAddress(const std::string& multicast_ip, std::uint16_t port, const unsigned int& ifindex = 0);
        
        FunctionReturn<> send(const std::vector<std::uint8_t>& data);

        static FunctionReturn<IPMulticastSender<T>> factory();

        IPMulticastSender(const IPMulticastSender&) = delete;
        IPMulticastSender& operator=(const IPMulticastSender&) = delete;

        IPMulticastSender(IPMulticastSender&& other)
            : sockFd{other.sockFd}, family{other.family}, targets{std::move(other.targets)} {
            other.sockFd = -1;
        }

        IPMulticastSender& operator=(const IPMulticastSender&& other) {
            if (this != &other) {
                if (this->sockFd >= 0) {
                    ::close(this->sockFd);
                }
                this->sockFd = other.sockFd;
                this->family = other.family;
                this->targets = std::move(other.targets);
                other.sockFd = -1;
            }

            return *this;
        }
    };

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<IPMulticastSender<T>> IPMulticastSender<T>::factory() {
        IPMulticastSender<T> sender;
        sender.sockFd = ::socket(sender.family, SOCK_DGRAM, 0);

        if (sender.sockFd < 0) {
            return FunctionReturn<IPMulticastSender<T>>{ExitCode::Error, "IPMulticastSender socket creation failed"};
        }

        // unsigned char loop = 0; 
        // if constexpr (std::is_same_v<T, ::sockaddr_in>) {
        //     if (::setsockopt(sender.sockFd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
        //         return FunctionReturn<IPMulticastSender<T>>{ExitCode::Error, "IPMulticastSender loopback = 0 setsockopt faield"};
        //     }
        // } else {
        //     if (::setsockopt(sender.sockFd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
        //         return FunctionReturn<IPMulticastSender<T>>{ExitCode::Error, "IPMulticastSender loopback = 0 setsockopt faield"};
        //     }
        // }

        return FunctionReturn<IPMulticastSender<T>>{std::move(sender)};
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    IPMulticastSender<T>::IPMulticastSender() {
        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            this->family = AF_INET;
        } else { // sockaddr_in6
            this->family = AF_INET6;
        }
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    IPMulticastSender<T>::~IPMulticastSender() {
        if (this->sockFd >= 0) {
            ::close(this->sockFd);
        }
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<> IPMulticastSender<T>::addMulticastAddress(const std::string& multicast_ip, std::uint16_t port, const unsigned int& ifindex) {
        Target t{};
        t.ifindex = ifindex;

        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            t.addr.sin_family = this->family;
            t.addr.sin_port = ::htons(port);
            if (::inet_pton(this->family, multicast_ip.c_str(), &t.addr.sin_addr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv4 {} from text to binary", multicast_ip)};
            }
        } else { // sockaddr_in6
            t.addr.sin6_family = this->family;
            t.addr.sin6_port = ::htons(port);
            if (::inet_pton(this->family, multicast_ip.c_str(), &t.addr.sin6_addr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv4 {} from text to binary", multicast_ip)};
            }
        }

        this->targets.push_back(t);
        return FunctionReturn<>{};
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<> IPMulticastSender<T>::removeMulticastAddress(const std::string& multicast_ip, std::uint16_t port, const unsigned int& ifindex) {
        Target t{};
        t.ifindex = ifindex;

        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            t.addr.sin_family = this->family;
            t.addr.sin_port = ::htons(port);
            if (::inet_pton(this->family, multicast_ip.c_str(), &t.addr.sin_addr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv4 {} from text to binary", multicast_ip)};
            }

            auto it = std::remove_if(this->targets.begin(), this->targets.end(),
                [&](const Target& existing) {
                    return existing.ifindex == t.ifindex &&
                        existing.addr.sin_port == t.addr.sin_port &&
                        existing.addr.sin_addr.s_addr == t.addr.sin_addr.s_addr;
                });
            if (it == this->targets.end()) {
                return FunctionReturn<>{ExitCode::Error, "Multicast IPv4 address not found"};
            }
            this->targets.erase(it, this->targets.end());
        } else {
            t.addr.sin6_family = this->family;
            t.addr.sin6_port = ::htons(port);
            if (::inet_pton(this->family, multicast_ip.c_str(), &t.addr.sin6_addr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv6 {} from text to binary", multicast_ip)};
            }

            auto it = std::remove_if(this->targets.begin(), this->targets.end(),
                [&](const Target& existing) {
                    return existing.ifindex == t.ifindex &&
                        existing.addr.sin6_port == t.addr.sin6_port &&
                        std::memcmp(&existing.addr.sin6_addr, &t.addr.sin6_addr, sizeof(::in6_addr)) == 0;
                });
            if (it == this->targets.end()) {
                return FunctionReturn<>{ExitCode::Error, "Multicast IPv6 address not found"};
            }
            this->targets.erase(it, this->targets.end());
        }

        return FunctionReturn<>{};
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<> IPMulticastSender<T>::send(const std::vector<std::uint8_t>& data) {
        for (auto& target : targets) {
            if (target.ifindex != 0) {
                if constexpr (std::is_same_v<T, ::sockaddr_in>) {
                    ::in_addr local_if{};
                    local_if.s_addr = htonl(INADDR_ANY);
                    if (::setsockopt(this->sockFd, IPPROTO_IP, IP_MULTICAST_IF, &local_if, sizeof(local_if)) < 0) {
                        return FunctionReturn<>{
                            std::format("Failed setting IPPROTO_IP, IP_MULTICAST_IF on {} socket for {} interface", this->sockFd, target.ifindex)
                        };
                    }
                } else {
                    if (::setsockopt(this->sockFd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &target.ifindex, sizeof(target.ifindex)) < 0) {
                        return FunctionReturn<>{
                            std::format("Failed setting IPPROTO_IPV6, IPV6_MULTICAST_IF on {} socket for {} interface", this->sockFd, target.ifindex)
                        };
                    }
                }
            }

            auto* sa = reinterpret_cast<::sockaddr*>(&target.addr);
            ::socklen_t salen = static_cast<socklen_t>(sizeof(target.addr));
            if (::sendto(this->sockFd, data.data(), data.size(), 0, sa, salen) < 0) {
                return FunctionReturn<>{
                    std::format("Failed sending data on {} socket for {} interface", this->sockFd, target.ifindex)
                };
            }
        }

        return FunctionReturn<>{};
    }

}

#endif