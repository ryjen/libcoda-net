#include "config.h"
#include <algorithm>
#include <cstring>
#include "socket_server.h"
#include "exception.h"

namespace arg3
{
    namespace net
    {
        socket_server::socket_server(int port, socket_factory *factory, int queueSize)
            : socket(port, queueSize), pollFrequency_(4), factory_(factory), backgroundThread_(nullptr)
        {
        }

        socket_server::socket_server(socket_server &&other)
            : socket(std::move(other)), pollFrequency_(other.pollFrequency_), factory_(other.factory_),
              backgroundThread_(std::move(other.backgroundThread_)),
              listeners_(std::move(other.listeners_)), sockets_(std::move(other.sockets_))
        {
            other.sock_ = INVALID;
            other.factory_ = NULL;
            other.backgroundThread_ = nullptr;
        }

        socket_server::~socket_server()
        {
            if (is_valid())
                stop();
        }

        socket_server &socket_server::operator=(socket_server && other)
        {

            socket::operator=(std::move(other));

            pollFrequency_ = other.pollFrequency_;

            factory_ = other.factory_;

            backgroundThread_ = std::move(other.backgroundThread_);

            sockets_ = std::move(other.sockets_);

            listeners_ = std::move(other.listeners_);

            other.sock_ = INVALID;
            other.factory_ = NULL;
            other.backgroundThread_ = nullptr;

            return *this;
        }

        bool socket_server::operator==(const socket_server &other)
        {
            return port_ == other.port_;
        }

        bool socket_server::operator!=(const socket_server &other)
        {
            return !operator==(other);
        }

        bool socket_server::is_valid() const
        {
            return socket::is_valid() && factory_ != NULL;
        }

        void socket_server::stop()
        {
            close();

            notify_stop();

            if (backgroundThread_ != nullptr)
            {
                if (backgroundThread_->joinable())
                    backgroundThread_->join();

                backgroundThread_ = nullptr;
            }
        }

        void socket_server::add_listener(socket_server_listener *listener)
        {
            listeners_.push_back(listener);
        }

        void socket_server::notify_poll()
        {
            on_poll();

            for (auto &listener : listeners_)
            {
                listener->on_poll(this);
            }
        }

        void socket_server::notify_start()
        {
            on_start();

            for (auto &listener : listeners_)
            {
                listener->on_start(this);
            }
        }

        void socket_server::notify_stop()
        {
            on_stop();

            for (auto &listener : listeners_)
            {
                listener->on_stop(this);
            }
        }

        void socket_server::start_in_background()
        {
            backgroundThread_ = make_shared<thread>(&socket_server::run, this);
        }

        bool socket_server::listen()
        {
            bool success = socket::listen();

            notify_start();

            return success;
        }

        void socket_server::start()
        {
            run();
        }

        void socket_server::on_poll()
        {}

        void socket_server::on_start()
        {}

        void socket_server::on_stop()
        {}

        void socket_server::check_connections(std::function<bool(std::shared_ptr<buffered_socket>)> delegate)
        {
            if (!is_valid() || sockets_.empty()) return;

            sockets_.erase(std::remove_if(sockets_.begin(), sockets_.end(), delegate), sockets_.end());
        }

        void socket_server::poll()
        {
            static struct timeval null_time = {0};

            fd_set in_set;
            fd_set out_set;
            fd_set err_set;
            int maxdesc = 0;

            if (!is_valid())
                return;

            FD_ZERO(&in_set);
            FD_ZERO(&out_set);
            FD_ZERO(&err_set);
            FD_SET(sock_, &in_set);

            maxdesc = sock_;

            // prepare for sockets for polling
            check_connections([&maxdesc, &in_set, &out_set, &err_set](std::shared_ptr<buffered_socket> c)
            {
                if (!c->is_valid()) return true;

                maxdesc = std::max(maxdesc, c->sock_);
                FD_SET(c->sock_, &in_set);
                FD_SET(c->sock_, &out_set);
                FD_SET(c->sock_, &err_set);
                return false;
            });

            // poll
            if (select(maxdesc + 1, &in_set, &out_set, &err_set, &null_time) < 0)
            {
                if (errno != EINTR)
                {
                    throw socket_exception(strerror(errno));
                }
            }

            // check for new connection
            if (is_valid() && FD_ISSET(sock_, &in_set))
            {
                sockaddr_in addr;

                auto sock = factory_->create_socket(this, accept(addr), addr);

                sock->set_non_blocking(true);

                sock->notify_connect();

                sockets_.push_back(sock);
            }

            /* check for freaky connections */
            check_connections([&in_set, &err_set, &out_set](std::shared_ptr<buffered_socket> c)
            {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &err_set))
                {
                    FD_CLR(c->sock_, &in_set);
                    FD_CLR(c->sock_, &out_set);

                    c->close();
                    return true;
                }
                return false;
            });

            /* read from all readable connections, removing failed sockets */
            check_connections([&in_set, &out_set](std::shared_ptr<buffered_socket> c)
            {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &in_set))
                {
                    if (!c->read_to_buffer())
                    {
                        FD_CLR(c->sock_, &out_set);

                        c->close();
                        return true;
                    }
                }

                return false;
            });

            // strategically placed
            notify_poll();

            /* write to all writable connections, removing failed sockets */
            check_connections([&](std::shared_ptr<buffered_socket> c)
            {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &out_set))
                {
                    if (c->has_output())
                    {
                        if (!c->write_from_buffer())
                        {
                            c->close();

                            return true;
                        }
                    }
                }
                return false;
            });
        }

        void socket_server::run()
        {
            struct timeval last_time;

            if (!is_valid())
            {
                if (!listen())
                    return;
            }

            gettimeofday(&last_time, NULL);

            while (is_valid())
            {
                struct timeval now_time;
                long secDelta;
                long usecDelta;

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
                        throw socket_exception(strerror(errno));
                    }
                }

                // check still valid after wait
                if (!is_valid())
                    return;

                gettimeofday(&last_time, NULL);

                poll();
            }
        }
    }
}

