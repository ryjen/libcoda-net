#include "config.h"
#include <cerrno>
#include <fstream>
#include <cstring>
#include "socket.h"
#include "exception.h"
#ifndef _WIN32
#include <fcntl.h>
#endif

using namespace std;

namespace arg3
{

    namespace net
    {

#ifndef _WIN32

        int closesocket(SOCKET socket)
        {
            return close(socket);
        }
#endif

        socket::socket() :
            sock_ ( INVALID )
#ifdef HAVE_LIBSSL
            , sslHandle_(NULL), sslContext_(NULL)
#endif
        {
            memset ( &addr_, 0, sizeof ( addr_ ) );
        }

        socket::socket(SOCKET sock, const sockaddr_storage &addr) : sock_(sock), addr_(addr)
#ifdef HAVE_LIBSSL
            , sslHandle_(NULL), sslContext_(NULL)
#endif
        {

        }

        socket::socket(socket &&other) : sock_(other.sock_), addr_(std::move(other.addr_))
#ifdef HAVE_LIBSSL
            , sslHandle_(other.sslHandle_), sslContext_(other.sslContext_)
#endif
        {
            other.sock_ = INVALID;
#ifdef HAVE_LIBSSL
            other.sslHandle_ = NULL;
            other.sslContext_ = NULL;
#endif
        }

        socket::socket(const std::string &host, const int port) : sock_(INVALID)
#ifdef HAVE_LIBSSL
            , sslHandle_(NULL), sslContext_(NULL)
#endif
        {
            memset ( &addr_, 0, sizeof ( addr_ ) );

            connect(host, port);
        }

        socket &socket::operator=(socket && other)
        {
            sock_ = other.sock_;
            addr_ = std::move(other.addr_);
#ifdef HAVE_LIBSSL
            sslHandle_ = other.sslHandle_;
            sslContext_ = other.sslContext_;
            other.sslHandle_ = NULL;
            other.sslContext_ = NULL;
#endif
            other.sock_ = INVALID;

            return *this;
        }

        socket::~socket()
        {
            close();
        }

        void socket::close()
        {

#ifdef HAVE_LIBSSL
            if (sslHandle_ != NULL)
            {
                SSL_shutdown (sslHandle_);
                SSL_free (sslHandle_);
                sslHandle_ = NULL;
            }
            // automatically freed above?
            // if (sslContext_ != NULL)
            // {
            //     SSL_CTX_free (sslContext_);
            //     sslContext_ = NULL;
            // }
#endif
            if (sock_ != INVALID)
            {
                closesocket(sock_);
                sock_ = INVALID;
            }
        }

        int socket::send ( const data_buffer &s, int flags )
        {
            if (s.empty()) return 0;
#ifdef HAVE_LIBSSL
            if (sslHandle_ != NULL)
                return SSL_write(sslHandle_, s.data(), s.size() );
#endif
            return ::send ( sock_, s.data(), s.size(), flags );
        }

        int socket::send( void *s, size_t len, int flags)
        {
            if (len == 0) return 0;

#ifdef HAVE_LIBSSL
            if (sslHandle_ != NULL)
                return SSL_write(sslHandle_, s, len );
#endif
#ifdef _WIN32
            return ::send(sock_, reinterpret_cast<const char *>(s), len, flags);
#else
            return ::send(sock_, s, len, flags);
#endif
        }

        bool socket::is_valid() const
        {
            return sock_ != INVALID;
        }

        SOCKET socket::raw_socket() const
        {
            return sock_;
        }

        const char *socket::ip() const
        {
            if (!is_valid()) return "invalid";

            if (addr_.ss_family == AF_INET)
            {
                sockaddr_in *addr4 = (struct sockaddr_in *) &addr_;

                return inet_ntoa(addr4->sin_addr);
            }
            else
            {
                static char straddr[INET6_ADDRSTRLEN] = {0};

                sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &addr_;

                return inet_ntop(AF_INET6, &addr6->sin6_addr, straddr, sizeof(straddr) - 1);
            }
        }

        int socket::port() const
        {
            if (!is_valid()) return INVALID;

            if (addr_.ss_family == AF_INET)
            {
                struct sockaddr_in *addr4 = (struct sockaddr_in *) &addr_;
                return ntohs(addr4->sin_port);
            }
            else
            {
                struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &addr_;
                return ntohs(addr6->sin6_port);
            }
        }

        int socket::recv(data_buffer &s)
        {
            unsigned char buf [ MAXRECV + 1 ] = {0};

            int status;

            memset ( buf, 0, sizeof(buf) );

            s.clear();

#ifdef HAVE_LIBSSL
            if (sslHandle_ != NULL)
                status = SSL_read( sslHandle_, buf, MAXRECV );
            else
#endif
                status = ::recv ( sock_, buf, MAXRECV, 0 );

            if (status > 0)
            {
                s.insert(s.end(), &buf[0], &buf[status]);

                on_recv(s);
            }

            return status;
        }

        void socket::on_recv(data_buffer &s) {}

        socket &socket::operator << ( const data_buffer &s )
        {
            if ( send ( s ) < 0)
            {
                throw socket_exception ( "Could not write to socket." );
            }

            return *this;

        }

        socket &socket::operator >> ( data_buffer &s )
        {
            if ( recv(s) < 0)
            {
                throw new socket_exception("Could not read from socket");
            }

            return *this;
        }

        bool socket::connect ( const string &host, const int port )
        {

            struct addrinfo hints, *result, *p;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
            hints.ai_socktype = SOCK_STREAM;

            char servnam[101] = {0};
            snprintf(servnam, 100, "%d", port);

            if (getaddrinfo(host.c_str(), servnam, &hints, &result) != 0)
                return false;

            if (is_valid())
            {
                close();
            }

            for (p = result; p != NULL; p = p->ai_next)
            {
                sock_ = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if (sock_ == INVALID) continue;

                if (::connect ( sock_, p->ai_addr, p->ai_addrlen ) != INVALID)
                    break;

                closesocket(sock_);
                sock_ = INVALID;
            }

            if (p == NULL || sock_ == INVALID)
            {
                freeaddrinfo(result);
                return false;
            }

            memmove(&addr_, p->ai_addr, p->ai_addrlen);

            freeaddrinfo(result);

#ifdef HAVE_LIBSSL
            if (sslHandle_ != NULL)
            {
                // Connect the SSL struct to our connection
                if (!SSL_set_fd (sslHandle_, sock_))
                    throw socket_exception (ERR_error_string(ERR_get_error(), NULL));

                // Initiate SSL handshake
                if (SSL_connect (sslHandle_) != 1)
                    throw socket_exception (ERR_error_string(ERR_get_error(), NULL));
            }
#endif
            return true;
        }

        bool socket::listen(const int port, const int backlogSize)
        {
            struct addrinfo hints, *result = NULL, *p = NULL;

            const int on = 1;

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

            char servnam[101] = {0};
            snprintf(servnam, 100, "%d", port);

            int r = getaddrinfo(NULL, servnam, &hints, &result);

            if (r != 0)
            {
                throw socket_exception( gai_strerror(r));
            }

            if (result == NULL)
            {
                throw socket_exception("unable to get address info");
            }

            for (p = result; p != NULL; p = p->ai_next)
            {
                sock_ = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol);

                if ( sock_ == INVALID )
                    continue;

                if ( setsockopt ( sock_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof ( on ) ) == -1 )
                    continue;

                if ( ::bind(sock_, p->ai_addr, p->ai_addrlen) == 0 )
                    break;

                closesocket(sock_);
                sock_ = INVALID;
            }

            if (p == NULL)
            {
                freeaddrinfo(result);
                return false;
            }

            memmove(&addr_, p->ai_addr, p->ai_addrlen);

            freeaddrinfo(result);

            int listen_return = ::listen ( sock_, backlogSize );

            if ( listen_return == -1 )
            {
                return false;
            }

            return true;
        }

        SOCKET socket::accept ( sockaddr_storage &addr) const
        {
            socklen_t addr_length = sizeof(addr);

            SOCKET sock = ::accept ( sock_, (struct sockaddr *) &addr, &addr_length );

            if ( sock <= 0 )
            {
                return false;
            }

            return sock;
        }


        void socket::set_non_blocking ( const bool b )
        {
#ifndef _WIN32
            int opts = fcntl ( sock_, F_GETFL );

            if ( opts < 0 )
            {
                return;
            }

            if ( b )
                opts = ( opts | O_NONBLOCK );
            else
                opts = ( opts & ~O_NONBLOCK );

            fcntl ( sock_, F_SETFL, opts );
#else
            ioctlsocket( sock_, FIONBIO, 0 );
#endif
        }

        bool socket::is_secure() const
        {
#ifdef HAVE_LIBSSL
            return sslHandle_ != NULL && sslContext_ != NULL;
#else
            return false;
#endif
        }
        void socket::set_secure(const bool b)
        {
#ifndef HAVE_LIBSSL
            throw socket_exception("SSL not supported");
#else
            if (!b)
            {
                // check if we need to free
                if (sslHandle_)
                {
                    SSL_shutdown (sslHandle_);
                    SSL_free (sslHandle_);
                    sslHandle_ = NULL;
                }
                // automatically freed above?
                // if (sslContext_)
                // {
                //     SSL_CTX_free (sslContext_);
                //     sslContext_ = NULL;
                // }

                return;
            }

            // check we're already initialized
            if (sslHandle_)
                return;

            if (is_valid())
                throw socket_exception("socket already initalized");

            static bool initialized = false;

            if (!initialized)
            {
                // Register the available ciphers and digests
                SSL_library_init ();
                // Register the error strings for libcrypto & libssl
                SSL_load_error_strings ();

                initialized = true;
            }

            // New context saying we are a client, and using SSL 2 or 3
            sslContext_ = SSL_CTX_new (SSLv23_client_method ());
            if (sslContext_ == NULL)
            {
                throw socket_exception (ERR_error_string(ERR_get_error(), NULL));
            }

            // Create an SSL struct for the connection
            sslHandle_ = SSL_new (sslContext_);

            if (sslHandle_ == NULL)
                throw socket_exception (ERR_error_string(ERR_get_error(), NULL));
#endif
        }
    }

}
