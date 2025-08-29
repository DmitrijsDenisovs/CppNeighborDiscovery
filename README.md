# Neighbor discoverer

Both programs share some headers placed in include directory.

Main daemon program is compiled by Makefile.daemon file, placed in build_daemon directory. (cpp_neighbor_discovery.out).

CLI list retrieval program is compiled by Makefile.cli file, placed in build_cli directory. (cpp_cli_neighbor_requestor).

Both programs include all headers and .cpp files in include, which might be suboptimal.

CLI returns network interfaces only with matching subnet/prefix IPs.

Daemon uses multicast sockets on IPv4 and IPv6 to send all of its network interface data over all available network interfaces.

Communication between CLI and daemon is done by a UNIX domain socket.

Shared configuration file is found in include/Config/Config.hpp.
