#pragma once
#ifndef IPMULTICASTRECEIVER_HPP
#define IPMULTICASTRECEIVER_HPP

#include "Utility/FunctionReturn.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <vector>
#include <string>
#include <format>
#include <type_traits>
#include <stdexcept>
#include <memory>

using Utility::FunctionReturn;
using Utility::ExitCode;
using Logging::LoggableFrom;
using Logging::ILogger;

namespace Network::Sockets {
    //templated socket handler for multicast message receival (::sockaddr_in for IPv4, ::sockaddr_in fo IPv6)
    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    class IPMulticastReceiver {
    private:
        std::uint16_t port;
        int sockFd;
        int family;

        explicit IPMulticastReceiver(std::uint16_t port);

    public:
        ~IPMulticastReceiver();

        FunctionReturn<> enableMulticastGroup(const std::string& multicast_ip, unsigned int ifindex = 0);
        FunctionReturn<> disableMulticastGroup(const std::string& multicast_ip, unsigned int ifindex = 0);

        ssize_t receive(std::vector<std::uint8_t>& buffer, T* sender = nullptr);

        static FunctionReturn<IPMulticastReceiver<T>> factory(std::uint16_t port);

        IPMulticastReceiver(const IPMulticastReceiver&) = delete;
        IPMulticastReceiver& operator=(const IPMulticastReceiver&) = delete;

        IPMulticastReceiver(IPMulticastReceiver&& other) 
            : port(other.port), sockFd(other.sockFd), family(other.family) {
            other.sockFd = -1;
        }

        IPMulticastReceiver& operator=(IPMulticastReceiver&& other) {
            if (this != &other) {
                if (this->sockFd >= 0) {
                    ::close(this->sockFd);
                }
                this->sockFd = other.sockFd;
                this->family = other.family;
                this->port = other.port;
                other.sockFd = -1;
            }
            return *this;
        }
    };

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<IPMulticastReceiver<T>> IPMulticastReceiver<T>::factory(std::uint16_t port) {
        IPMulticastReceiver<T> receiver{port};

        receiver.sockFd = ::socket(receiver.family, SOCK_DGRAM | SOCK_NONBLOCK, 0);
        if (receiver.sockFd < 0) {
            return FunctionReturn<IPMulticastReceiver<T>>{ExitCode::Error, std::format("IPMulticastReceiver socket creation on port {} failed", port)};
        }

        int reuse = 1;
        if (::setsockopt(receiver.sockFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            ::close(receiver.sockFd);
            return FunctionReturn<IPMulticastReceiver<T>>{ExitCode::Error, std::format("setsockopt SO_REUSEADDR on IPMulticastReceiver socket on port {} failed", port)};
        }

        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            ::sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
            addr.sin_port = ::htons(receiver.port);
            if (::bind(receiver.sockFd, reinterpret_cast<::sockaddr*>(&addr), sizeof(addr)) < 0) {
                ::close(receiver.sockFd);
                return FunctionReturn<IPMulticastReceiver<T>>{ExitCode::Error, std::format("bind IPv4 on IPMulticastReceiver socket on port {} failed", port)};
            }
        } else {
            ::sockaddr_in6 addr{};
            addr.sin6_family = AF_INET6;
            addr.sin6_addr = ::in6addr_any;
            addr.sin6_port = ::htons(receiver.port);
            if (::bind(receiver.sockFd, reinterpret_cast<::sockaddr*>(&addr), sizeof(addr)) < 0) {
                ::close(receiver.sockFd);
                return FunctionReturn<IPMulticastReceiver<T>>{ExitCode::Error, std::format("bind IPv6 on IPMulticastReceiver socket on port {} failed", port)};
            }
        }

        return FunctionReturn<IPMulticastReceiver<T>>{std::move(receiver)};
    }


    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    IPMulticastReceiver<T>::IPMulticastReceiver(std::uint16_t port) 
        : port{port}, sockFd{-1} {
        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            this->family = AF_INET;
        } else {
            this->family = AF_INET6;
        }
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    IPMulticastReceiver<T>::~IPMulticastReceiver() {
        if (this->sockFd >= 0) {
            ::close(this->sockFd);
        }
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<> IPMulticastReceiver<T>::enableMulticastGroup(const std::string& multicast_ip, unsigned int ifindex) {
        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            ::ip_mreq mreq{};
            if (::inet_pton(AF_INET, multicast_ip.c_str(), &mreq.imr_multiaddr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv4 {} from text to binary", multicast_ip)};
            }
            mreq.imr_interface.s_addr = ::htonl(INADDR_ANY);
            if (::setsockopt(this->sockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                return FunctionReturn<>{std::format("Failed setting IPPROTO_IP, IP_DROP_MEMBERSHIP on {} socket", this->sockFd)};
            }
        } else {
            ::ipv6_mreq mreq{};
            if (::inet_pton(AF_INET6, multicast_ip.c_str(), &mreq.ipv6mr_multiaddr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv6 {} from text to binary", multicast_ip)};
            }
            mreq.ipv6mr_interface = ifindex;
            if (::setsockopt(this->sockFd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                return FunctionReturn<>{std::format("Failed setting IPPROTO_IPV5, IPV6_DROP_MEMBERSHIP on {} socket", this->sockFd)};
            }
        }

        return FunctionReturn<>{};
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    FunctionReturn<> IPMulticastReceiver<T>::disableMulticastGroup(const std::string& multicast_ip, unsigned int ifindex) {
        if constexpr (std::is_same_v<T, ::sockaddr_in>) {
            ::ip_mreq mreq{};
            if (::inet_pton(AF_INET, multicast_ip.c_str(), &mreq.imr_multiaddr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv4 {} from text to binary", multicast_ip)};
            }
            mreq.imr_interface.s_addr = ::htonl(INADDR_ANY);
            if (::setsockopt(this->sockFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                return FunctionReturn<>{std::format("Failed setting IPPROTO_IP, IP_DROP_MEMBERSHIP on {} socket", this->sockFd)};
            }
        } else {
            ::ipv6_mreq mreq{};
            if (::inet_pton(AF_INET6, multicast_ip.c_str(), &mreq.ipv6mr_multiaddr) < 0) {
                return FunctionReturn<>{std::format("Failed converting IPv6 {} from text to binary", multicast_ip)};
            }
            mreq.ipv6mr_interface = ifindex;    
            if (::setsockopt(this->sockFd, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
                return FunctionReturn<>{std::format("Failed setting IPPROTO_IPV5, IPV6_DROP_MEMBERSHIP on {} socket", this->sockFd)};
            }
        }

        return FunctionReturn<>{};
    }

    template<typename T>
    requires (std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>)
    ssize_t IPMulticastReceiver<T>::receive(std::vector<std::uint8_t>& buffer, T* sender) {
        socklen_t len = sender ? sizeof(T) : 0;
        ssize_t n = ::recvfrom(sockFd, buffer.data(), buffer.size(), 0, 
            sender ? reinterpret_cast<::sockaddr*>(sender) : nullptr, sender ? &len : nullptr);
        return n;
    }
}

#endif