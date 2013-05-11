#include "socketserver.h"
#include "exception.h"
#include "../log/log.h"

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

            sockets_.erase(std::remove_if(sockets_.begin(), sockets_.end(), delegate), sockets_.end());
        }

        SocketServer::SocketServer(int port, SocketFactory *factory, int queueSize)
            : Socket(port, queueSize), pollFrequency_(4), factory_(factory)
        {}

        SocketServer::SocketServer(const SocketServer &other)
            : Socket(other), pollFrequency_(other.pollFrequency_), factory_(other.factory_)
        {}

        SocketServer::SocketServer(SocketServer &&other)
            : Socket(std::move(other)), pollFrequency_(other.pollFrequency_), factory_(std::move(other.factory_))
        {
            other.sock_ = INVALID;
            other.factory_ = NULL;
        }

        SocketServer::~SocketServer()
        {}

        SocketServer &SocketServer::operator=(const SocketServer &other)
        {
            if(this != &other)
            {
                Socket::operator=(other);

                pollFrequency_ = other.pollFrequency_;

                factory_ = other.factory_;
            }

            return *this;
        }

        SocketServer &SocketServer::operator=(SocketServer &&other)
        {
            if(this != &other)
            {
                Socket::operator=(std::move(other));

                pollFrequency_ = other.pollFrequency_;

                factory_ = std::move(other.factory_);
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

                factory_->run([&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;
                    maxdesc = std::max(maxdesc, c.sock_);
                    FD_SET(c.sock_, &in_set);
                    FD_SET(c.sock_, &out_set);
                    FD_SET(c.sock_, &exc_set);
                    return false;
                });

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

                    BufferedSocket *sock = factory_->createSocket(accept(addr), addr);

                    sock->set_non_blocking(true);
                }

                /* check for freaky connections */
                factory_->run([&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &exc_set))
                    {
                        FD_CLR(c.sock_, &in_set);
                        FD_CLR(c.sock_, &out_set);

                        c.close();
                        return true;
                    }
                    return false;
                });

                /* read from all connections, removing failed sockets */
                factory_->run([&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &in_set))
                    {
                        if (!c.readToBuffer())
                        {
                            FD_CLR(c.sock_, &out_set);

                            c.close();
                            return true;
                        }
                    }

                    return false;
                });

                /* write to all connections, removing failed sockets */
                factory_->run([&](BufferedSocket &c) {
                    if(!c.is_valid()) return true;

                    if (FD_ISSET(c.sock_, &out_set))
                    {
                        if(c.hasOutput())
                        {
                            if(!c.writeFromBuffer())
                            {
                                c.close();

                                return true;
                            }
                        }
                    }
                    return false;
                });

            }
        }

    }
}