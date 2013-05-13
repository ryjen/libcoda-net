#ifndef THIN

#include "socketfactory.h"

namespace arg3
{
    namespace net
    {
        DefaultSocketFactory defaultSocketFactory;

        BufferedSocket* DefaultSocketFactory::createSocket(SOCKET sock, const sockaddr_in &addr)
        {
            BufferedSocket connection(sock, addr);

            connections_.push_back(connection);

            return &connections_[connections_.size()-1];
        }

        vector<BufferedSocket>& DefaultSocketFactory::getSockets()
        {
            return connections_;
        }

        void SocketFactory::run(std::function<bool(BufferedSocket &)> delegate)
        {
            vector<BufferedSocket> &sockets_ = getSockets();

            // run the delegate, remove connection if delegate returns false
            sockets_.erase(std::remove_if(sockets_.begin(), sockets_.end(), delegate), sockets_.end());
        }

    }
}

#endif
