
#include "include/Config/Config.hpp"
#include "include/Logging/StdLogger.hpp"
#include "include/Network/NetworkNeighborDiscoverer.hpp"
#include "include/Network/DiscoverySettings.hpp"
#include "include/Unix/UnixDomainSettings.hpp"
#include "include/Processes/Process.hpp"

#include <functional>

using Logging::StdLogger;
using Network::DiscoverySettings;
using Unix::UnixDomainSettings;
using Network::NetworkNeighborDiscoverer;
using Processes::Process;

int main() {
    auto logger = Logging::StdLogger::getInstance();

    logger->info("Service started");
    
    //used for comm with other daemons over net
    DiscoverySettings netSettings;
    netSettings.port = Config::PORT;
    netSettings.sendingPeriodS = Config::SENDING_PERIOD_SECONDS;
    netSettings.neighborActivityPeriodS = Config::NEIGHBOR_ACTIVITY_PERIOD_SECONDS;
    netSettings.maxBufferSize = Config::SINGLE_MESSAGE_MAX_SIZE_BYTES;

    //used for comm with cli
    UnixDomainSettings localCommSettings;
    localCommSettings.requestString = Config::UNIX_DOMAIN_REQUEST_COMMAND;
    localCommSettings.maxBufferSize = Config::SINGLE_MESSAGE_MAX_SIZE_BYTES;
    localCommSettings.socketPath = Config::UNIX_DOMAIN_SOCKET_PATH;

    NetworkNeighborDiscoverer discoverer{logger, netSettings, localCommSettings};

    Process& process = 
        Process::create(true, Config::ITERATION_PERIOD_SECONDS, Config::STD_REDIRECT_PATH, std::bind(&NetworkNeighborDiscoverer::runIteration, &discoverer));

    process.daemonize();
    process.run();

    return 0;
}