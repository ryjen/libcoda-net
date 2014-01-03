
#include <cerrno>
#include <fstream>
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
            sock_ ( INVALID ), references_(NULL), backlogSize_(BACKLOG_SIZE), port_(0)
        {
            memset ( &addr_, 0, sizeof ( addr_ ) );
        }

        socket::socket(SOCKET sock, const sockaddr_in &addr) : sock_(sock), addr_(addr),
            references_(new unsigned(0)), backlogSize_(BACKLOG_SIZE), port_(0)
        {

        }

        socket::socket(const socket &sock) : sock_(sock.sock_), addr_(sock.addr_), references_(sock.references_),
            backlogSize_(sock.backlogSize_), port_(sock.port_)
        {
            update_reference_count();
        }

        void socket::update_reference_count()
        {
            if (references_)
            {
                (*references_)++;
            }
            else if (sock_ != INVALID)
            {
                references_ = new unsigned(0);
                (*references_)++;
            }
        }

        socket::socket(socket &&other) : sock_(other.sock_), addr_(std::move(other.addr_)), references_(std::move(other.references_)),
            backlogSize_(other.backlogSize_), port_(other.port_)
        {
            other.sock_ = INVALID;
            other.references_ = NULL;
        }

        socket &socket::operator=(const socket &other)
        {
            if (this != &other)
            {
                sock_ = other.sock_;
                addr_ = other.addr_;
                backlogSize_ = other.backlogSize_;
                port_ = other.port_;

                update_reference_count();
            }
            return *this;
        }

        socket &socket::operator=(socket && other)
        {
            if (this != &other)
            {
                sock_ = other.sock_;
                addr_ = std::move(other.addr_);
                references_ = std::move(other.references_);
                backlogSize_ = other.backlogSize_;
                port_ = other.port_;

                other.sock_ = INVALID;
                other.references_ = NULL;
            }
            return *this;
        }


        socket::~socket()
        {
            if ( is_valid())
            {
                if (!references_ || !*references_)
                {
                    close();
                    if (references_)
                    {
                        delete references_;
                        references_ = NULL;
                    }
                }
                else
                {
                    (*references_)--;
                }
            }
        }

        void socket::close()
        {
            closesocket(sock_);
            sock_ = INVALID;
        }

        int socket::send ( const data_buffer &s, int flags )
        {
            return ::send ( sock_, &s[0], s.size(), flags );
        }

        int socket::send( void *s, size_t len, int flags)
        {
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
            return is_valid() ? inet_ntoa(addr_.sin_addr) : "invalid";
        }

        int socket::port() const
        {
            return port_ == 0 ? addr_.sin_port : port_;
        }
        void socket::set_port(const int port)
        {
            port_ = port;
        }
        void socket::set_ip(const string &ip)
        {
#ifdef WIN32
            addr_.sin_addr.s_addr = inet_addr(ip);
#else
            inet_aton(ip.c_str(), &addr_.sin_addr);
#endif
        }

        int socket::recv(data_buffer &s)
        {

            unsigned char buf [ MAXRECV + 1 ];

            memset ( buf, 0, sizeof(buf) );

            s.clear();

            int status = ::recv ( sock_, buf, MAXRECV, 0 );

            if (status > 0)
            {
                s.insert(s.end(), &buf[0], &buf[status]);
            }

            return status;
        }

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

        socket::socket(const std::string &host, const int port) : socket()
        {
            connect(host, port);
        }

        bool socket::connect ( const string &host, const int port )
        {
            if ( ! is_valid() ) return false;

            struct hostent *hp;

            if ((hp = gethostbyname(host.c_str())) == 0)
                return false;

            addr_.sin_family = AF_INET;
            addr_.sin_port = htons ( port );
            addr_.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;

            int status = ::connect ( sock_, ( sockaddr * ) &addr_, sizeof ( addr_ ) );

            if ( status == 0 )
            {
                port_ = port;
                return true;
            }
            else
                return false;
        }


        socket::socket(int port, int queueSize) : sock_(INVALID), references_(NULL), backlogSize_(queueSize), port_(port)
        {}

        bool socket::create()
        {
#ifdef _WIN32
            static int started = 0;
            if (!started)
            {
                short wVersionRequested = 0x101;
                WSADATA wsaData;
                if (WSAStartup( wVersionRequested, &wsaData ) == -1)
                {
                    return false;
                }
                if (wsaData.wVersion != 0x101)
                {
                    return false;
                }
                started = 1;
            }
#endif

            sock_ = ::socket ( AF_INET, SOCK_STREAM, 0 );

            if ( ! is_valid() )
                return false;


            // TIME_WAIT - argh
            int on = 1;
            if ( setsockopt ( sock_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof ( on ) ) == -1 )
                return false;


            return true;

        }

        bool socket::bind ( )
        {
            if ( ! is_valid() )
            {
                return false;
            }

            addr_.sin_family = AF_INET;
            addr_.sin_addr.s_addr = INADDR_ANY;
            addr_.sin_port = htons ( port_ );

            int bind_return = ::bind ( sock_, ( struct sockaddr * ) &addr_, sizeof ( addr_ ) );


            if ( bind_return == -1 )
            {
                return false;
            }

            return true;
        }

        bool socket::listen()
        {
            if ( ! create() )
            {
                throw socket_exception ( "Could not create server socket." );
            }

            if ( ! bind ( ) )
            {
                throw socket_exception ( "Could not bind to port." );
            }

            if ( ! is_valid() )
            {
                return false;
            }

            int listen_return = ::listen ( sock_, backlogSize_ );

            if ( listen_return == -1 )
            {
                return false;
            }

            return true;
        }

        SOCKET socket::accept (sockaddr_in &addr) const
        {
            int addr_length = sizeof ( addr );

            SOCKET sock = ::accept ( sock_, ( sockaddr * ) &addr, ( socklen_t * ) &addr_length );

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
    }

}
