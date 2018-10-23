
#include "socket.h"
#include <cerrno>
#include <cstring>
#include <fstream>
#include "exception.h"
#include "secure_layer.h"
#ifndef _WIN32
#include <fcntl.h>
#endif

using namespace std;

namespace coda
{
    namespace net
    {
#ifndef _WIN32
        int closesocket(SOCKET socket)
        {
            return close(socket);
        }
#endif

        socket::socket() noexcept : sock_(INVALID), non_blocking_(false), ssl_(nullptr)
        {
            memset(&addr_, 0, sizeof(addr_));
        }


        socket::socket(SOCKET sock, const sockaddr_storage &addr) noexcept
            : sock_(sock), addr_(addr), non_blocking_(false), ssl_(nullptr)
        {
        }

        socket::socket(socket &&other) noexcept
            : sock_(other.sock_),
              addr_(std::move(other.addr_)),
              non_blocking_(other.non_blocking_),
              ssl_(std::move(other.ssl_))
        {
            other.sock_ = INVALID;
            other.ssl_ = nullptr;
        }

        socket::socket(const std::string &host, const int port, bool secure)
            : sock_(INVALID), non_blocking_(false), ssl_(nullptr)
        {
            memset(&addr_, 0, sizeof(addr_));

            set_secure(secure);

            connect(host, port);
        }

        socket &socket::operator=(socket &&other) noexcept
        {
            sock_ = other.sock_;
            addr_ = std::move(other.addr_);
            non_blocking_ = other.non_blocking_;
            ssl_ = other.ssl_;
            other.sock_ = INVALID;
            other.ssl_ = nullptr;

            return *this;
        }

        socket::~socket()
        {
            close();
        }

        bool socket::operator==(const socket &other) const noexcept
        {
            return sock_ == other.sock_;
        }

        void socket::close()
        {
            if (ssl_) {
                ssl_->shutdown();
            }

            if (sock_ != INVALID) {
                shutdown(sock_, SHUT_RDWR);
                closesocket(sock_);
                sock_ = INVALID;
            }
            non_blocking_ = false;
        }

        int socket::send(const data_buffer &s, int flags)
        {
            return this->send(s.data(), s.size(), flags);
        }

        int socket::send(const void *s, size_t len, int flags)
        {
            if (!is_valid()) {
                return INVALID;
            }

            if (len == 0 || s == NULL) {
                return 0;
            }

            if (ssl_) {
                return ssl_->send(s, len);
            }

            return ::send(sock_, s, len, flags);
        }

        bool socket::is_valid() const noexcept
        {
            return sock_ != INVALID;
        }

        SOCKET socket::raw_socket() const noexcept
        {
            return sock_;
        }

        const char *socket::ip() const
        {
            if (!is_valid()) {
                return "invalid";
            }

            if (addr_.ss_family == AF_INET) {
                sockaddr_in *addr4 = (struct sockaddr_in *)&addr_;

                return inet_ntoa(addr4->sin_addr);
            }

            if (addr_.ss_family == AF_INET6) {
                static char straddr[INET6_ADDRSTRLEN] = {0};

                sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr_;

                return inet_ntop(AF_INET6, &addr6->sin6_addr, straddr, sizeof(straddr) - 1);
            }

            return "unknown";
        }

        int socket::port() const
        {
            if (!is_valid()) {
                return INVALID;
            }

            if (addr_.ss_family == AF_INET) {
                struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr_;
                return ntohs(addr4->sin_port);
            } else {
                struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr_;
                return ntohs(addr6->sin6_port);
            }
        }

        int socket::recv(data_buffer &s, int flags)
        {
            if (!is_valid()) {
                return INVALID;
            }

            unsigned char buf[MAXRECV + 1] = {0};

            s.clear();

            int status;

            if (ssl_) {
                status = ssl_->read(buf, MAXRECV);
            } else {
                status = ::recv(sock_, buf, MAXRECV, flags);
            }

            if (status > 0) {
                s.insert(s.end(), &buf[0], &buf[status]);

                on_recv(s);
            }

            return status;
        }

        void socket::on_recv(data_buffer &s)
        {
        }

        socket &socket::operator<<(const data_buffer &s)
        {
            if (send(s) < 0) {
                throw socket_exception("Could not write to socket.");
            }

            return *this;
        }

        socket &socket::operator>>(data_buffer &s)
        {
            if (recv(s) < 0) {
                throw new socket_exception("Could not read from socket");
            }

            return *this;
        }

        bool socket::connect(const string &host, const int port)
        {
            struct addrinfo hints, *result, *p;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;  // use AF_INET6 to force IPv6
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            char servnam[101] = {0};
            snprintf(servnam, 100, "%d", port);

            if (getaddrinfo(host.c_str(), servnam, &hints, &result) != 0) {
                return false;
            }

            if (is_valid()) {
                close();
            }

            for (p = result; p != NULL; p = p->ai_next) {
                auto sock = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (sock == INVALID) {
                    continue;
                }

                if (::connect(sock, p->ai_addr, p->ai_addrlen) != INVALID) {
                    sock_ = sock;
                    break;
                }

                closesocket(sock);
            }

            if (p == NULL || sock_ == INVALID) {
                freeaddrinfo(result);
                return false;
            }

            memmove(&addr_, p->ai_addr, p->ai_addrlen);

            freeaddrinfo(result);

            if (ssl_) {
                ssl_->attach(sock_);
            }

            return true;
        }

        bool socket::listen(const int port, const int backlogSize)
        {
            struct addrinfo hints, *result = NULL, *p = NULL;

            const int on = 1;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */

            char servnam[101] = {0};
            snprintf(servnam, 100, "%d", port);

            int r = getaddrinfo(NULL, servnam, &hints, &result);

            if (r != 0) {
                throw socket_exception(gai_strerror(r));
            }

            if (result == NULL) {
                throw socket_exception("unable to get address info");
            }

            for (p = result; p != NULL; p = p->ai_next) {
                auto sock = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (sock == INVALID) {
                    continue;
                }

                if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
                    continue;
                }

                if (::bind(sock, p->ai_addr, p->ai_addrlen) == 0) {
                    sock_ = sock;
                    break;
                }

                closesocket(sock);
            }

            if (p == NULL || sock_ == INVALID) {
                freeaddrinfo(result);
                return false;
            }

            memmove(&addr_, p->ai_addr, p->ai_addrlen);

            freeaddrinfo(result);

            int listen_return = ::listen(sock_, backlogSize);

            if (listen_return == -1) {
                return false;
            }

            if (ssl_) {
                ssl_->attach(sock_);
            }

            return true;
        }

        SOCKET socket::accept(sockaddr_storage &addr) const
        {
            socklen_t addr_length = sizeof(addr);

            SOCKET sock = ::accept(sock_, (struct sockaddr *)&addr, &addr_length);

            if (sock <= 0) {
                return INVALID;
            }

            return sock;
        }


        void socket::set_non_blocking(const bool b)
        {
#ifndef _WIN32
            int opts = fcntl(sock_, F_GETFL);

            if (opts < 0) {
                return;
            }

            if (b) {
                opts = (opts | O_NONBLOCK);
            } else {
                opts = (opts & ~O_NONBLOCK);
            }

            fcntl(sock_, F_SETFL, opts);
#else
            ioctlsocket(sock_, FIONBIO, b ? 1 : 0);
#endif

            non_blocking_ = b;
        }

        bool socket::is_non_blocking() const noexcept
        {
            return non_blocking_;
        }

        bool socket::is_secure() const noexcept
        {
            return ssl_ != nullptr;
        }

        void socket::set_secure(bool value)
        {
            if (value) {
#ifdef OPENSSL_FOUND
                ssl_ = std::make_shared<openssl_layer>();
#endif
            } else {
                ssl_ = nullptr;
            }
        }
    }
}
