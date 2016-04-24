#include "polling_socket_server.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include "exception.h"

namespace arg3
{
    namespace net
    {
        polling_socket_server::polling_socket_server(const factory_type &factory) : socket_server(factory), pollFrequency_(DEFAULT_POLL_FREQUENCY)
        {
        }


        polling_socket_server::polling_socket_server(polling_socket_server &&other)
            : socket_server(std::move(other)), pollFrequency_(other.pollFrequency_)
        {
        }

        polling_socket_server &polling_socket_server::operator=(polling_socket_server &&other)
        {
            socket_server::operator=(std::move(other));

            pollFrequency_ = other.pollFrequency_;

            return *this;
        }

        void polling_socket_server::on_start()
        {
            set_non_blocking(true);
        }

        void polling_socket_server::notify_poll()
        {
            on_poll();

            std::lock_guard<std::recursive_mutex> lock(listeners_mutex_);

            for (const auto &listener : listeners_) {
                auto poll_listener = std::dynamic_pointer_cast<polling_socket_server_listener>(listener);

                if (poll_listener) {
                    poll_listener->on_poll(this);
                }
            }
        }

        void polling_socket_server::check_connections(std::function<bool(const socket_type &)> delegate)
        {
            if (!is_valid()) return;

            std::lock_guard<std::recursive_mutex> lock(sockets_mutex_);

            sockets_.erase(std::remove_if(std::begin(sockets_), std::end(sockets_), delegate), std::end(sockets_));
        }

        void polling_socket_server::poll()
        {
            static struct timeval null_time = {0};

            fd_set in_set, out_set, err_set;
            int maxdesc = sock_;

            if (!is_valid()) return;

            FD_ZERO(&in_set);
            FD_ZERO(&out_set);
            FD_ZERO(&err_set);
            FD_SET(sock_, &in_set);

            // prepare for sockets for polling
            check_connections([&](const socket_type &c) {
                if (!c || !c->is_valid()) return true;

                maxdesc = std::max(maxdesc, c->sock_);
                FD_SET(c->sock_, &in_set);
                FD_SET(c->sock_, &out_set);
                FD_SET(c->sock_, &err_set);
                return false;
            });

            // poll
            if (select(maxdesc + 1, &in_set, &out_set, &err_set, &null_time) < 0) {
                if (errno != EINTR) {
                    throw socket_exception(strerror(errno));
                }
            }

            // check for new connection
            if (FD_ISSET(sock_, &in_set)) {
                sockaddr_storage addr;

                auto sock = factory_->create_socket(this, accept(addr), addr);

                sock->set_non_blocking(true);

                sock->notify_connect();

                add_socket(sock);
            }

            /* check for freaky connections */
            check_connections([&](const socket_type &c) {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &err_set)) {
                    FD_CLR(c->sock_, &in_set);
                    FD_CLR(c->sock_, &out_set);

                    c->close();

                    return true;
                }
                return false;
            });

            /* read from all readable connections, removing failed sockets */
            check_connections([&](const socket_type &c) {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &in_set)) {
                    if (!c->read_to_buffer()) {
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
            check_connections([&](const socket_type &c) {
                if (!c->is_valid()) return true;

                if (FD_ISSET(c->sock_, &out_set)) {
                    if (c->has_output()) {
                        if (!c->write_from_buffer()) {
                            c->close();
                            return true;
                        }
                    }
                }
                return false;
            });
        }

        void polling_socket_server::wait_for_poll(struct timeval *last_time)
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            if (last_time == NULL || pollFrequency_ <= 0) {
                return;
            }

            gettimeofday(&now_time, NULL);

            usecDelta = ((int)last_time->tv_usec) - ((int)now_time.tv_usec) + 1000000 / pollFrequency_;
            secDelta = ((int)last_time->tv_sec) - ((int)now_time.tv_sec);

            while (usecDelta < 0) {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000) {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            // check if server should stall for a moment based on poll frequency
            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;

                if (select(0, NULL, NULL, NULL, &stall_time) == -1) {
                    throw socket_exception(strerror(errno));
                }
            }

            gettimeofday(last_time, NULL);
        }

        void polling_socket_server::run()
        {
            struct timeval last_time;

            gettimeofday(&last_time, NULL);

            assert(sockets_.empty());

            while (is_valid()) {
                if (pollFrequency_ > 0) {
                    wait_for_poll(&last_time);
                }

                poll();
            }
        }
    }
}
