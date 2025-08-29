#pragma once
#ifndef NETWORKNEIGHBORDISCOVERY_HPP
#define NETWORKNEIGHBORDISCOVERY_HPP

#include "DiscoverySettings.hpp"
#include "Unix/UnixDomainSettings.hpp"
#include "Unix/UnixSocket.hpp"
#include "Logging/ILogger.hpp"
#include "Logging/LoggableFrom.hpp"
#include "Containers/IndexedTimedSet.hpp"
#include "NetInterfaces/NetInterface.hpp"
#include "Sockets/IPMulticastSender.hpp"
#include "Sockets/IPMulticastReceiver.hpp"

#include <memory>
#include <cstdint>
#include <string>
#include <type_traits>
#include <chrono>

using Network::DiscoverySettings;
using Unix::UnixDomainSettings;
using Unix::UnixSocket;
using Logging::LoggableFrom;
using Logging::ILogger;
using Containers::IndexedTimedSet;
using Network::NetInterfaces::NetInterface;
using Network::Sockets::IPMulticastReceiver;
using Network::Sockets::IPMulticastSender;

namespace Network {
    class NetworkNeighborDiscoverer : public LoggableFrom { 
    private:
        const DiscoverySettings settings;
        const UnixDomainSettings localSettings;

        std::chrono::steady_clock::time_point prevSendTime;

        IndexedTimedSet<std::string, NetInterface> neighbors{};
        std::vector<NetInterface> prevNifs{};

        std::unique_ptr<IPMulticastSender<::sockaddr_in6>> ipv6sender = nullptr;
        std::unique_ptr<IPMulticastSender<::sockaddr_in>> ipv4sender = nullptr;

        std::unique_ptr<IPMulticastReceiver<::sockaddr_in6>> ipv6receiver = nullptr;
        std::unique_ptr<IPMulticastReceiver<::sockaddr_in>> ipv4receiver = nullptr;

        std::unique_ptr<UnixSocket> unixDomainServer = nullptr;

    public:
        NetworkNeighborDiscoverer(std::shared_ptr<ILogger> logger, const DiscoverySettings& settings, const UnixDomainSettings& localSettings)
            : LoggableFrom{logger}, settings{settings}, localSettings{localSettings}
        {
            setupIPv4Sockets();
            setupIPv6Sockets();
            setupUnixDomainSockets();
        }

        void runIteration();

        void setupIPv4Sockets();
        void setupIPv6Sockets();
        void setupUnixDomainSockets();
    };

}

#endif
