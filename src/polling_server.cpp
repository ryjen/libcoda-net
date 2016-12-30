#include "polling_server.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include "exception.h"

namespace rj
{
    namespace net
    {
        namespace polling
        {
            server::server(const factory_type &factory) : socket_server(factory), frequency_(DEFAULT_POLL_FREQUENCY)
            {
            }


            server::server(server &&other) : socket_server(std::move(other)), frequency_(other.frequency_)
            {
            }

            server &server::operator=(server &&other)
            {
                socket_server::operator=(std::move(other));

                frequency_ = other.frequency_;

                return *this;
            }

            void server::on_start()
            {
                set_non_blocking(true);
            }

            void server::notify_poll()
            {
                on_poll();

                std::lock_guard<std::recursive_mutex> lock(listeners_mutex_);

                for (const auto &listener : listeners_) {
                    auto poll_listener = std::dynamic_pointer_cast<server_listener>(listener);

                    if (poll_listener) {
                        poll_listener->on_poll(this);
                    }
                }
            }

            void server::check_connections(std::function<bool(const socket_type &)> delegate)
            {
                if (!is_valid()) return;

                std::lock_guard<std::recursive_mutex> lock(sockets_mutex_);

                sockets_.erase(std::remove_if(std::begin(sockets_), std::end(sockets_), delegate), std::end(sockets_));
            }

            void server::poll()
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

                    maxdesc = std::max(maxdesc, c->raw_socket());
                    FD_SET(c->raw_socket(), &in_set);
                    FD_SET(c->raw_socket(), &out_set);
                    FD_SET(c->raw_socket(), &err_set);
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

                    auto sock = accept(addr);

                    if (sock != INVALID) {
                        on_accept(sock, addr);
                    }
                }

                /* check for freaky connections */
                check_connections([&](const socket_type &c) {
                    if (!c->is_valid()) return true;

                    if (FD_ISSET(c->raw_socket(), &err_set)) {
                        FD_CLR(c->raw_socket(), &in_set);
                        FD_CLR(c->raw_socket(), &out_set);

                        c->close();

                        return true;
                    }
                    return false;
                });

                /* read from all readable connections, removing failed sockets */
                check_connections([&](const socket_type &c) {
                    if (!c->is_valid()) return true;

                    if (FD_ISSET(c->raw_socket(), &in_set)) {
                        if (!c->read_to_buffer()) {
                            FD_CLR(c->raw_socket(), &out_set);

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

                    if (FD_ISSET(c->raw_socket(), &out_set)) {
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

            void server::wait_for_poll(struct timeval *last_time)
            {
                struct timeval now_time;
                long secDelta;
                long usecDelta;

                if (last_time == NULL || frequency_ <= 0) {
                    return;
                }

                gettimeofday(&now_time, NULL);

                usecDelta = ((int)last_time->tv_usec) - ((int)now_time.tv_usec) + 1000000 / frequency_;
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

            void server::on_poll()
            {
            }

            void server::run()
            {
                struct timeval last_time;

                gettimeofday(&last_time, NULL);

                while (is_valid()) {
                    wait_for_poll(&last_time);

                    poll();
                }
            }
        }
    }
}
