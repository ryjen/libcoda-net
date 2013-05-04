#include "socketserver.h"
#include "exception.h"
#include "../log/log.h"

namespace arg3
{
    namespace net
    {
        SocketServer::SocketServer(int port, int queueSize)
            : Socket(port, queueSize), pollFrequency_(4)
        {}

         SocketServer::SocketServer(const SocketServer &other)
            : Socket(other), pollFrequency_(other.pollFrequency_)
        {}

        SocketServer::SocketServer(SocketServer &&other)
            : Socket(std::move(other)), pollFrequency_(other.pollFrequency_)
        {}

        SocketServer::~SocketServer()
        {}
        
        SocketServer &SocketServer::operator=(const SocketServer &other)
        {
            if(this != &other)
            {
                Socket::operator=(other);

                pollFrequency_ = other.pollFrequency_;
            }

            return *this;
        }

        SocketServer &SocketServer::operator=(SocketServer &&other)
        {
            if(this != &other)
            {
                Socket::operator=(std::move(other));

                pollFrequency_ = other.pollFrequency_;
            }

            return *this;
        }

        void SocketServer::start(bool inBackground)
        {
            if(inBackground)
                listenThread_ = thread(&SocketServer::loop, this);
            else
                loop();
        }

        void SocketServer::stop()
        {
            close();

            listenThread_.join();
        }

        void SocketServer::addListener(SocketServerListener *listener)
        {
            listeners_.push_back(listener);
        }

        void SocketServer::notifyConnect(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onConnect(sock);
            }
        }

        void SocketServer::notifyWillRead(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onWillRead(sock);
            }
        }

        void SocketServer::notifyDidRead(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onDidRead(sock);
            }
        }

        void SocketServer::notifyWillWrite(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onWillWrite(sock);
            }
        }

        void SocketServer::notifyDidWrite(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onDidWrite(sock);
            }
        }

        void SocketServer::notifyClose(BufferedSocket &sock)
        {
            for(auto &l : listeners_)
            {
                l->onClose(sock);
            }
        }

        void SocketServer::loop()
        {
            fd_set in_set;
            fd_set out_set;
            fd_set exc_set;

            struct timeval last_time;

            if(!is_valid())
                listen();

            gettimeofday(&last_time, NULL);

            while(is_valid())
            {
                static struct timeval null_time;
                struct timeval now_time;
                long secDelta;
                long usecDelta;
                int maxdesc = 0;

                gettimeofday(&now_time, NULL);

                usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec) + 1000000 / pollFrequency_;
                secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);

                while (usecDelta < 0)
                {
                    usecDelta += 1000000;
                    secDelta -= 1;
                }

                while (usecDelta >= 1000000)
                {
                    usecDelta -= 1000000;
                    secDelta += 1;
                }

                if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
                {
                    struct timeval stall_time;

                    stall_time.tv_usec = usecDelta;
                    stall_time.tv_sec = secDelta;

                    if (select(0, NULL, NULL, NULL, &stall_time) == -1)
                    {
                        throw SocketException("stall");
                    }

                    // check still valid after wait
                    if(!is_valid())
                        break;
                }

                gettimeofday(&last_time, NULL);

                FD_ZERO(&in_set);
                FD_ZERO(&out_set);
                FD_ZERO(&exc_set);
                FD_SET(sock_, &in_set);

                maxdesc = sock_;

                connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                [&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;
                    maxdesc = std::max(maxdesc, c.sock_);
                    FD_SET(c.sock_, &in_set);
                    FD_SET(c.sock_, &out_set);
                    FD_SET(c.sock_, &exc_set);
                    return false;
                }), connections_.end());

                if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
                {
                    if (errno != EINTR)
                    {
                        throw SocketException(strerror(errno));
                    }
                }

                if (FD_ISSET(sock_, &in_set))
                {
                    sockaddr_in addr;

                    BufferedSocket sock(accept(addr), addr);

                    sock.set_non_blocking(true);

                    notifyConnect(sock);

                    connections_.push_back(sock);
                }

                /* check for freaky connections */
                connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                [&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &exc_set))
                    {
                        FD_CLR(c.sock_, &in_set);
                        FD_CLR(c.sock_, &out_set);

                        notifyClose(c);

                        c.close();
                        return true;
                    }
                    return false;
                }), connections_.end());

                /* read from all connections, removing failed sockets */
                connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                [&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &in_set))
                    {
                        notifyWillRead(c);

                        if (!c.readToBuffer())
                        {
                            FD_CLR(c.sock_, &out_set);

                            notifyClose(c);

                            c.close();
                            return true;
                        }

                        notifyDidRead(c);
                    }

                    return false;
                }), connections_.end());

                /* write to all connections, removing failed sockets */
                connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                [&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &out_set))
                    {
                        if(c.hasOutput())
                        {
                            notifyWillWrite(c);

                            if(!c.writeFromBuffer())
                            {
                                notifyClose(c);

                                c.close();

                                return true;
                            }

                            notifyDidWrite(c);
                        }
                    }
                    return false;
                }), connections_.end());

            }
        }

    }
}