#ifndef RJ_NET_SERVER_ASYNC_CLIENT_H
#define RJ_NET_SERVER_ASYNC_CLIENT_H

#include "../socket_server.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            class client
            {
               public:
                virtual void run() = 0;
            };

            class default_client : public buffered_socket, public client
            {
               public:
                using buffered_socket::buffered_socket;
                using buffered_socket::operator=;
                void run();
            };
        }
    }
}

#endif
