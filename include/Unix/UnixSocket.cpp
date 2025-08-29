#include "UnixSocket.hpp"

#include "Utility/FunctionReturn.hpp"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <cstring>
#include <vector>

using Unix::UnixSocket;
using Utility::FunctionReturn;
using Utility::ExitCode;

FunctionReturn<UnixSocket> UnixSocket::serverFactory(const std::string& path) {
    ::unlink(path.c_str());

    int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        return FunctionReturn<UnixSocket>{ExitCode::Error, "socket(AF_UNIX, SOCK_STREAM) failed"};
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return FunctionReturn<UnixSocket>{ExitCode::Error, "bind() failed"};
    }

    if (::listen(fd, SOMAXCONN) < 0) {
        ::close(fd);
        return {ExitCode::Error, "listen() failed"};
    }

    return FunctionReturn<UnixSocket>{ UnixSocket{fd, path, true} };
}

// accepts a single client and returns a connected UnixSocket
FunctionReturn<UnixSocket> UnixSocket::acceptClient() {
    if (!this->isServer || this->sockFd < 0) {
        return {ExitCode::Error, "acceptClient() called on non-server socket"};
    }

    int cfd = ::accept(this->sockFd, nullptr, nullptr);
    if (cfd < 0) {
        return {ExitCode::Error, "accept() failed"};
    }

    // client connection doesn't own a path
    return FunctionReturn<UnixSocket>{ UnixSocket{cfd, "", false} };
}

// connect to a UNIX domain socket
FunctionReturn<UnixSocket> UnixSocket::clientFactory(const std::string& path) {
    int fd = ::socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        return {ExitCode::Error, "socket(AF_UNIX, SOCK_STREAM) failed"};
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return {ExitCode::Error, "connect() failed"};
    }

    return FunctionReturn<UnixSocket>{ UnixSocket{fd, "", false} };
}


// sends entire buffer;
FunctionReturn<> UnixSocket::send(const std::vector<std::uint8_t>& buff) {
    if (this->sockFd < 0) {
        return {ExitCode::Error, "invalid socket"};
    }
    const std::uint8_t* p = buff.data();
    size_t left = buff.size();
    while (left > 0) {
        ssize_t n = ::write(this->sockFd, p, left);
        if (n < 0) {
            //try again if interrupted
            if (errno == EINTR) {
                continue;
            } 
            return {ExitCode::Error, "write() failed"};
        } else if (n == 0) {
            return {ExitCode::Error, "write() returned 0"};
        }
        p += n;
        left -= static_cast<size_t>(n);
    }
    return {ExitCode::Ok};
}

//sends string (command)
FunctionReturn<> UnixSocket::send(const std::string& s) {
    if (this->sockFd < 0) {
        return {ExitCode::Error, "invalid socket"};
    }
    const char* p = s.data();
    size_t left = s.size();
    while (left > 0) {
        ssize_t n = ::write(sockFd, p, left);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return {ExitCode::Error, "write() failed"};
        }
        if (n == 0) return {ExitCode::Error, "write() returned 0"};
        p += n;
        left -= static_cast<size_t>(n);
    }
    return {};
}

// Receive up to maxBytes into a std::string (blocking once)
FunctionReturn<std::string> UnixSocket::receiveString(std::size_t maxBytes) {
    if (this->sockFd < 0) {
        return {ExitCode::Error, "invalid socket"};
    }

    std::string out;
    out.resize(maxBytes);
    ssize_t n = ::read(sockFd, out.data(), out.size());
    if (n < 0) {
        if (errno == EINTR) {
            return receiveString(maxBytes);
        }
        return {ExitCode::Error, "read() failed"};
    }
    if (n == 0) {
        out.clear();
        return {std::move(out)};
    }
    out.resize(static_cast<std::size_t>(n));
    return {std::move(out)};
}

// receive up to maxBytes into a byte vector (blocking once)
FunctionReturn<void> UnixSocket::receive(std::vector<std::uint8_t>& buff) {
    if (this->sockFd < 0) {
        return {ExitCode::Error, "invalid socket"};
    }

    ssize_t n = ::read(this->sockFd, buff.data(), buff.size());

    if (n < 0) {
        if (errno == EINTR) {
            // retry once
            return this->receive(buff);
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            buff.clear();
            return {ExitCode::Ok};
        }
        return {ExitCode::Error, "read() failed: " + std::string(::strerror(errno))};
    } else if (n == 0) {
        buff.clear();
        return {ExitCode::Ok};
    }

    buff.resize(static_cast<std::size_t>(n));
    return {ExitCode::Ok};
}