#ifndef ARG3_NET_SOCKET_CLIENT_H_
#define ARG3_NET_SOCKET_CLIENT_H_

#include <thread>
#include "buffered_socket.h"

namespace arg3
{
    namespace net
    {
        class socket_client : public buffered_socket
        {
           public:
            using buffered_socket::buffered_socket;
            virtual ~socket_client();
            void start();

           private:
            void run();
            std::shared_ptr<std::thread> thread_;
        };
    }
}

#endif
