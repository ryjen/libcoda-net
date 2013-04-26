#include "serversocket.h"
#include "exception.h"

namespace arg3
{
    namespace net
    {

        ServerSocket::ServerSocket(int queueSize) : backlogSize_(queueSize)
        {}

        ServerSocket::ServerSocket(const ServerSocket &other) : Socket(other), backlogSize_(other.backlogSize_)
        {}

        ServerSocket& ServerSocket::operator=(const ServerSocket &other)
        {
            Socket::operator=(other);

            if(this != &other) {
                backlogSize_ = other.backlogSize_;
            }

            return *this;
        }

        void ServerSocket::start ( int port )
        {
            if ( ! create() )
            {
                throw SocketException ( "Could not create server socket." );
            }

            if ( ! bind (port) )
            {
                throw SocketException ( "Could not bind to port." );
            }

            listen();
        }

        ServerSocket::~ServerSocket()
        {
        }

        bool ServerSocket::create()
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

            sock_ = socket ( AF_INET, SOCK_STREAM, 0 );

            if ( ! is_valid() )
                return false;


            // TIME_WAIT - argh
            int on = 1;
            if ( setsockopt ( sock_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof ( on ) ) == -1 )
                return false;


            return true;

        }



        bool ServerSocket::bind ( int port )
        {
            if ( ! is_valid() )
            {
                return false;
            }

            addr_.sin_family = AF_INET;
            addr_.sin_addr.s_addr = INADDR_ANY;
            addr_.sin_port = htons ( port );

            int bind_return = ::bind ( sock_, ( struct sockaddr * ) &addr_, sizeof ( addr_ ) );


            if ( bind_return == -1 )
            {
                return false;
            }

            return true;
        }

        bool ServerSocket::listen() const
        {
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

        bool ServerSocket::accept (Socket &s) const
        {
            sockaddr_in addr;

            int addr_length = sizeof ( addr );

            int sock = ::accept ( sock_, ( sockaddr * ) &addr, ( socklen_t * ) &addr_length );

            if ( sock <= 0 )
            {
                return false;
            }

            s.sock_ = sock;
            s.addr_ = addr;

            return true;
        }
    }
}