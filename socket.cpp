
#include <cerrno>
#include <fstream>
#ifndef _WIN32
#include <fcntl.h>
#endif
#include "socket.h"
#include "exception.h"

using namespace std;

namespace arg3
{

    namespace net {

#ifndef _WIN32

        int closesocket(SOCKET socket)
        {
            return close(socket);
        }
#endif

        Socket::Socket() :
            sock_ ( -1 )
        {
            memset ( &addr_, 0, sizeof ( addr_ ) );
        }

        Socket::Socket(SOCKET sock, const sockaddr_in &addr) : sock_(sock), addr_(addr)
        {

        }

        Socket::Socket(const Socket &sock) : sock_(sock.sock_), addr_(sock.addr_)
        {

        }

        Socket &Socket::operator=(const Socket &other)
        {
            if(this != &other) {
                sock_ = other.sock_;
                addr_ = other.addr_;
            }
            return *this;
        }

        Socket::~Socket()
        {
            if ( is_valid())
            {
                close();
            }
        }

        void Socket::close()
        {
            closesocket(sock_);
            sock_ = INVALID;
        }

        int Socket::send ( const string &s, int flags )
        {
            return ::send ( sock_, s.c_str(), s.size(), flags );
        }

        int Socket::send( unsigned const char *s, size_t len, int flags)
        {
#ifdef _WIN32
            return ::send(sock_, reinterpret_cast<const char *>(s), len, flags);
#else
            return ::send(sock_, s, len, flags);
#endif
        }

        bool Socket::is_valid() const
        {
            return sock_ != INVALID;
        }

        SOCKET Socket::getSocket() const {
            return sock_;
        }

        const char *Socket::getIP() const
        {
            return is_valid() ? inet_ntoa(addr_.sin_addr) : "invalid";
        }

        int Socket::getPort() const
        {
            return addr_.sin_port;
        }
        void Socket::setPort(const int port)
        {
            addr_.sin_port = port;
        }
        void Socket::setIP(const string &ip)
        {
#ifdef WIN32
            addr_.sin_addr.s_addr = inet_addr(ip);
#else
            inet_aton(ip.c_str(), &addr_.sin_addr);
#endif
        }

        int Socket::recv(string &s)
        {

            char buf [ MAXRECV + 1 ];

            memset ( buf, 0, sizeof(buf) );

            s = "";

            int status = ::recv ( sock_, buf, MAXRECV, 0 );

            if (status > 0)
            {
                s = buf;
            }

            return status;
        }

        const Socket& Socket::operator << ( const string& s )
        {
            if ( send ( s ) < 0)
            {
                throw SocketException ( "Could not write to socket." );
            }

            return *this;

        }

        const Socket& Socket::operator >> ( string& s )
        {
            if ( recv(s) < 0)
            {
                throw new SocketException("Could not read from socket");
            }

            return *this;
        }


        void Socket::set_non_blocking ( const bool b )
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

            fcntl ( sock_, F_SETFL,opts );
#else
            ioctlsocket( sock_, FIONBIO, 0 );
#endif
        }

    }

}
