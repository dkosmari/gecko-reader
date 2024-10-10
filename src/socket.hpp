// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <chrono>
#include <stdexcept>

#include <network.h>


namespace net {


    struct error : std::runtime_error {

        error(const char* msg, int status);

    };


    struct init_guard {
        init_guard();
        ~init_guard();
    };


    struct socket {

        int fd = -1;


        constexpr
        socket() noexcept = default;

        explicit
        constexpr
        socket(int fd)
            noexcept :
            fd{fd}
        {}

        socket(int domain,
               int style,
               int protocol = 0);

        socket(socket&& other);

        socket&
        operator =(socket&& other)
            noexcept;

        ~socket();

        void
        close();

        operator bool() const noexcept;

        void
        bind(const struct sockaddr& addr,
             socklen_t size);

        void
        bind(const struct sockaddr_in& addr);

        void
        listen(unsigned backlog);

        socket
        accept(struct sockaddr& addr,
               socklen_t& size);

        socket
        accept(struct sockaddr_in& addr);

        int
        send(const char* buf,
             int size,
             unsigned flags = 0);

        void
        send_all(const char* buf,
                 int size,
                 unsigned flags = 0);

        int
        recv(char* buf, int size,
             unsigned flags = 0);

        bool
        is_readable(std::chrono::milliseconds ms = std::chrono::milliseconds{0})
            const;

    };


} // namespace net


#endif
