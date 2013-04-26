#include "socket.h"

namespace arg3
{
    namespace net
    {

        class ClientSocket : public Socket
        {
        public:
            ClientSocket();
            ClientSocket(const std::string &host, const int port);
            ClientSocket(const ClientSocket &other);
            virtual ~ClientSocket();
            ClientSocket& operator=(const ClientSocket &other);

            // Client initialization
            bool connect ( const std::string &host, const int port );
        };


    }
}