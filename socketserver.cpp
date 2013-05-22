#ifndef THIN

#include "socketserver.h"
#include "exception.h"

namespace arg3
{
    namespace net
    {
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

        bool SocketServer::operator==(const SocketServer &other)
        {
            return port_ == other.port_;
        }

        bool SocketServer::operator!=(const SocketServer &other)
        {
            return !operator==(other);
        }

        void SocketServer::addListener(SocketServerListener *listener)
        {
            listeners_.push_back(listener);
        }

        void SocketServer::notifyPoll()
        {
            onPoll();

            for(auto &listener : listeners_)
            {
                listener->onPoll(this);
            }
        }

        void SocketServer::notifyStart()
        {
            onStart();

            for(auto &listener : listeners_)
            {
                listener->onStart(this);
            }
        }

        void SocketServer::notifyStop()
        {
            onStop();

            for(auto &listener : listeners_)
            {
                listener->onStop(this);
            }
        }

        void SocketServer::start()
        {
            listenThread_ = thread(&SocketServer::loop, this);
        }

        void SocketServer::stop()
        {
            close();

            listenThread_.join();
        }

        void SocketServer::onPoll()
        {}

        void SocketServer::onStart()
        {}

        void SocketServer::onStop()
        {}

        void SocketServer::foreach(std::function<bool(std::shared_ptr<BufferedSocket>)> delegate)
        {
            sockets_.erase(std::remove_if(sockets_.begin(), sockets_.end(), delegate), sockets_.end());
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

            onStart();

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

                // check if server should stall for a moment based on poll frequency
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

                // prepare for sockets for polling
                foreach([&](std::shared_ptr<BufferedSocket> c) {
                    if(!c->is_valid()) return true;
                    maxdesc = std::max(maxdesc, c->sock_);
                    FD_SET(c->sock_, &in_set);
                    FD_SET(c->sock_, &out_set);
                    FD_SET(c->sock_, &exc_set);
                    return false;
                });

                // poll
                if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
                {
                    if (errno != EINTR)
                    {
                        throw SocketException(strerror(errno));
                    }
                }

                // check for new connection
                if (FD_ISSET(sock_, &in_set))
                {
                    sockaddr_in addr;

                    auto sock = factory_->createSocket(accept(addr), addr);

                    sock->set_non_blocking(true);

                    sock->notifyConnect();

                    sockets_.push_back(sock);
                }

                /* check for freaky connections */
                foreach([&](std::shared_ptr<BufferedSocket> c) {
                    if(!c->is_valid()) return true;

                    if (FD_ISSET(c->sock_, &exc_set))
                    {
                        FD_CLR(c->sock_, &in_set);
                        FD_CLR(c->sock_, &out_set);

                        c->close();
                        return true;
                    }
                    return false;
                });

                /* read from all readable connections, removing failed sockets */
                foreach([&](std::shared_ptr<BufferedSocket> c) {
                    if(!c->is_valid()) return true;

                    if (FD_ISSET(c->sock_, &in_set))
                    {
                        if (!c->readToBuffer())
                        {
                            FD_CLR(c->sock_, &out_set);

                            c->close();
                            return true;
                        }
                    }

                    return false;
                });

                notifyPoll();

                /* write to all writable connections, removing failed sockets */
                foreach([&](std::shared_ptr<BufferedSocket> c) {
                    if(!c->is_valid()) return true;

                    if (FD_ISSET(c->sock_, &out_set))
                    {
                        if(c->hasOutput())
                        {
                            if(!c->writeFromBuffer())
                            {
                                c->close();

                                return true;
                            }
                        }
                    }
                    return false;
                });

            }

            onStop();

        }
    }
}

#endif
