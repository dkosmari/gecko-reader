// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <cstring>              // strerror()
#include <stdexcept>
#include <string>

#include "socket.hpp"


using namespace std::literals;

using std::runtime_error;


namespace net {


    error::error(const char* msg,
                 int status) :
        runtime_error{msg + ": "s + std::strerror(-status)}
    {}


    init_guard::init_guard()
    {
        auto r = net_init();
        if (r)
            throw error{"net_init() failed", r};
    }


    init_guard::~init_guard()
        noexcept
    {
        net_deinit();
    }


    socket::socket(int domain,
                   int style,
                   int protocol) :
        socket{net_socket(domain,
                          style,
                          protocol)}
    {
        if (fd < 0)
            throw error{"net_socket() failed", fd};
    }


    socket::socket(socket&& other) :
        fd{other.fd}
    {
        other.fd = -1;
    }


    socket&
    socket::operator =(socket&& other)
        noexcept
    {
        if (this != &other) {
            close();
            fd = other.fd;
            other.fd = -1;
        }
        return *this;
    }


    socket::~socket()
    {
        close();
    }


    void
    socket::close()
    {
        if (fd != -1) {
            net_close(fd);
            fd = -1;
        }
    }


    socket::operator bool()
        const noexcept
    {
        return fd != -1;
    }


    void
    socket::bind(const struct sockaddr& addr,
                 socklen_t size)
    {
        int r = net_bind(fd, const_cast<struct sockaddr*>(&addr),
                         size);
        if (r < 0)
            throw error{"net_bind() failed", r};
    }


    void
    socket::bind(const struct sockaddr_in& addr)
    {
        bind(reinterpret_cast<const struct sockaddr&>(addr),
             sizeof addr);
    }


    void
    socket::listen(unsigned backlog)
    {
        int r = net_listen(fd, backlog);
        if (r < 0)
            throw error{"net_listen() failed", r};
    }


    socket
    socket::accept(struct sockaddr& addr,
                   socklen_t& size)
    {
        int r = net_accept(fd, &addr, &size);
        if (r < 0)
            throw error{"net_accept() failed", r};
        return socket{r};
    }


    socket
    socket::accept(struct sockaddr_in& addr)
    {
        socklen_t size = sizeof addr;
        auto result = accept(reinterpret_cast<struct sockaddr&>(addr),
                             size);
        if (addr.sin_family != AF_INET)
            throw runtime_error{"net::socket::accept(): address should be AF_INET but it's "s
                                + std::to_string(unsigned{addr.sin_family})};
        return result;
    }


    int
    socket::send(const char* buf,
                 int size,
                 unsigned flags)
    {
        int r = net_send(fd, buf, size, flags);
        if (r < 0)
            throw error{"net_send() failed: ", r};
        return r;
    }


    void
    socket::send_all(const char* buf,
                     int size,
                     unsigned flags)
    {
        int remain = size;
        while (remain > 0) {
            int sent = send(buf, size, flags);
            if (!sent)
                throw runtime_error{"net::socket::send_all(): connection lost."};
            remain -= sent;
        }
    }


    int
    socket::recv(char* buf,
                 int size,
                 unsigned flags)
    {
        int r = net_recv(fd, buf, size, flags);
        if (r < 0)
            throw error{"net_recv() failed", r};
        return r;
    }


    bool
    socket::is_readable(std::chrono::milliseconds ms)
        const
    {
        std::array<pollsd, 1> fds{};
        fds[0].socket = fd;
        fds[0].events = POLLIN;

        int r = net_poll(fds.data(), fds.size(), ms.count());
        if (r < 0)
            throw error{"net_poll() failed", r};
        if (r == 1 && fds[0].revents & POLLIN)
            return true;
        return false;
    }

} // namespace net
