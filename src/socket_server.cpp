#include "config.h"
#include <algorithm>
#include <cstring>
#include "socket_server.h"
#include "exception.h"
#include <cassert>

namespace arg3
{
    namespace net
    {
        socket_server::socket_server(socket_factory *factory)
            : pollFrequency_(4), factory_(factory), backgroundThread_(nullptr)
        {
        }

        socket_server::socket_server(socket_server &&other)
            : socket(std::move(other)), pollFrequency_(other.pollFrequency_), factory_(other.factory_),
              backgroundThread_(std::move(other.backgroundThread_)), sockets_(std::move(other.sockets_)),
              listeners_(std::move(other.listeners_))
        {
            other.sock_ = INVALID;
            other.factory_ = NULL;
            other.backgroundThread_ = nullptr;
        }

        socket_server::~socket_server()
        {
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
            if (!is_valid() || !other.is_valid())
                return false;

            return port() == other.port();
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

            sockets_mutex_.lock();
            sockets_.clear();
            sockets_mutex_.unlock();

            if (backgroundThread_ != nullptr)
            {
                if (backgroundThread_->joinable())
                    backgroundThread_->join();

                backgroundThread_ = nullptr;
            }
        }

        void socket_server::add_listener(socket_server_listener *listener)
        {
            listeners_mutex_.lock();

            if (find(listeners_.begin(), listeners_.end(), listener) == listeners_.end())
                listeners_.push_back(listener);

            listeners_mutex_.unlock();
        }

        void socket_server::set_socket_factory(socket_factory *factory)
        {
            factory_ = factory;
        }

        void socket_server::notify_poll()
        {
            on_poll();

            listeners_mutex_.lock();

            for (auto listener : listeners_)
            {
                listener->on_poll(this);
            }

            listeners_mutex_.unlock();
        }

        void socket_server::notify_start()
        {
            on_start();

            listeners_mutex_.lock();
            for (auto listener : listeners_)
            {
                listener->on_start(this);
            }
            listeners_mutex_.unlock();
        }

        void socket_server::notify_stop()
        {
            on_stop();

            listeners_mutex_.lock();
            for (auto listener : listeners_)
            {
                listener->on_stop(this);
            }
            listeners_mutex_.unlock();
        }

        bool socket_server::listen(const int port, const int backlogSize)
        {
            bool success = socket::listen(port, backlogSize);

            if (success)
                notify_start();

            return success;
        }

        void socket_server::start_in_background(int port, int backlogSize)
        {

            if (is_valid())
            {
                throw socket_exception("server already started");
            }

            if (!listen(port, backlogSize))
            {
                throw socket_exception("unable to listen on port");
            }

            backgroundThread_ = make_shared<thread>(&socket_server::run, this);
        }

        void socket_server::start(int port, int backlogSize)
        {
            if (is_valid())
            {
                throw socket_exception("server already started");
            }

            if (!listen(port, backlogSize))
            {
                throw socket_exception("unable to listen on port");
            }

            run();
        }

        void socket_server::on_poll()
        {}

        void socket_server::on_start()
        {}

        void socket_server::on_stop()
        {}

        void socket_server::check_connections(std::function<bool(const std::shared_ptr<buffered_socket> &)> delegate)
        {
            if (!is_valid()) return;

            sockets_mutex_.lock();
            sockets_.erase(std::remove_if(std::begin(sockets_), std::end(sockets_), delegate), std::end(sockets_));
            sockets_mutex_.unlock();
        }

        void socket_server::poll()
        {
            static struct timeval null_time = {0};

            fd_set in_set, out_set, err_set;
            int maxdesc = sock_;

            if (!is_valid())
                return;

            FD_ZERO(&in_set);
            FD_ZERO(&out_set);
            FD_ZERO(&err_set);
            FD_SET(sock_, &in_set);

            // prepare for sockets for polling
            check_connections([&](const std::shared_ptr<buffered_socket> &c)
            {
                if (!c || !c->is_valid()) return true;

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

                sockets_mutex_.lock();
                sockets_.push_back(sock);
                sockets_mutex_.unlock();
            }

            /* check for freaky connections */
            check_connections([&](const std::shared_ptr<buffered_socket> &c)
            {
                if (!c || !c->is_valid()) return true;

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
            check_connections([&](const std::shared_ptr<buffered_socket> &c)
            {
                if (!c || !c->is_valid()) return true;

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
            check_connections([&](const std::shared_ptr<buffered_socket> &c)
            {
                if (!c || !c->is_valid()) return true;

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

            gettimeofday(&last_time, NULL);

            assert(sockets_.empty());

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

                    // check still valid after wait
                    if (!is_valid())
                        break;
                }

                gettimeofday(&last_time, NULL);

                poll();
            }
        }
    }
}

