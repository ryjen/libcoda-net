#ifndef THIN

#include "socketfactory.h"

namespace arg3
{
    namespace net
    {
        DefaultSocketFactory defaultSocketFactory;

        std::shared_ptr<BufferedSocket> DefaultSocketFactory::createSocket(SocketServer *server, SOCKET sock, const sockaddr_in &addr)
        {
            return std::shared_ptr<BufferedSocket>(new BufferedSocket(sock, addr));
        }
    }
}

#endif
