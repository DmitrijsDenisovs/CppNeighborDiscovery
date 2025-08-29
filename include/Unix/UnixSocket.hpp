#pragma once
#ifndef UNIXSOCKET_HPP
#define UNIXOCKET_HPP

#include "Utility/FunctionReturn.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <cstring>
#include <vector>
#include <cstdint>

using Utility::FunctionReturn;
using Utility::ExitCode;

namespace Unix {
    //represents socket over unix domain, splits into server and client, server accepts requests and sends out data to requestors
    class UnixSocket {
    private:
        int sockFd{-1};
        std::string path;  
        bool isServer{false}; 

        //used by factories and accept()
        UnixSocket(int fd, std::string path, bool isServer)
            : sockFd{fd}, path{std::move(path)}, isServer{isServer} {}

        explicit UnixSocket(const std::string&) = delete;
        
    public:
        UnixSocket(const UnixSocket&) = delete;
        UnixSocket& operator=(const UnixSocket&) = delete;

        UnixSocket(UnixSocket&& other) noexcept
            : sockFd{other.sockFd}, path{std::move(other.path)}, isServer{other.isServer} {
            other.sockFd = -1;
            other.isServer = false;
        }

        UnixSocket& operator=(UnixSocket&& other) noexcept {
            if (this != &other) {
                if (this->sockFd >= 0) {
                    ::close(this->sockFd);
                }
                if (isServer && !path.empty()) {
                    ::unlink(this->path.c_str());
                }

                sockFd = other.sockFd;
                this->path = std::move(other.path);
                this->isServer = other.isServer;
                other.sockFd = -1;
                other.isServer = false;
            }
            return *this;
        }

        ~UnixSocket() {
            if (this->sockFd >= 0) {
                ::close(this->sockFd);
            }
            if (this->isServer && !this->path.empty()) {
                ::unlink(this->path.c_str());
            }
        }

        // creates a listening UNIX domain socket
        static FunctionReturn<UnixSocket> serverFactory(const std::string& path);

        // accepts a single client and returns a connected UnixSocket
        FunctionReturn<UnixSocket> acceptClient();

        // connect to a UNIX domain socket
        static FunctionReturn<UnixSocket> clientFactory(const std::string& path);

        FunctionReturn<> send(const std::vector<std::uint8_t>& buff);

        FunctionReturn<> send(const std::string& s);

        FunctionReturn<std::string> receiveString(std::size_t maxBytes = 64);

        FunctionReturn<void> receive(std::vector<std::uint8_t>& buff);

    };

} 

#endif