#include "clientsocket.h"

using namespace std;

namespace arg3
{
    namespace net
    {
        ClientSocket::ClientSocket()
        {

        }
        ClientSocket::ClientSocket(const std::string &host, const int port)
        {
            connect(host, port);
        }

        ClientSocket::ClientSocket(const ClientSocket &other) : Socket(other)
        {}

        ClientSocket::~ClientSocket() {}
        ClientSocket& ClientSocket::operator=(const ClientSocket &other)
        {
            Socket::operator=(other);

            return *this;
        }

        bool ClientSocket::connect ( const string &host, const int port )
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
                return true;
            else
                return false;
        }


    }
}