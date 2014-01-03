#ifndef ARG3_NET_TELNET_FACTORY_H_
#define ARG3_NET_TELNET_FACTORY_H_

#include "../socketfactory.h"

namespace arg3
{
    namespace net
    {
        class telnet_factory : public socket_factory
        {
        public:
            std::shared_ptr<buffered_socket> create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr);
        };
    }
}

#endif
