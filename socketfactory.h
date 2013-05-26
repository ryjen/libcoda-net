#ifndef ARG3_NET_SOCKETFACTORY_H
#define ARG3_NET_SOCKETFACTORY_H

#ifndef THIN

#include "bufferedsocket.h"

#include <memory>

namespace arg3
{
    namespace net
    {
        class BufferedSocket;
        class SocketServer;

        /* factory class to control sockets in a server */
        class SocketFactory
        {
        public:
            /* creates a buffered socket from a raw socket */
            virtual std::shared_ptr<BufferedSocket> createSocket(SocketServer *server, SOCKET sock, const sockaddr_in &addr)= 0;
        };

        /* default implementation of a socket factory */
        class DefaultSocketFactory : public SocketFactory
        {
        public:
            virtual std::shared_ptr<BufferedSocket> createSocket(SocketServer *server, SOCKET sock, const sockaddr_in &addr);
        };

        /* default factory instance */
        extern DefaultSocketFactory defaultSocketFactory;
    }
}

#endif
#endif
