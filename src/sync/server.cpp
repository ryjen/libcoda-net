#include "server.h"
#include <algorithm>
#include <cassert>
#include <cstring>

#include "../exception.h"
#include "listener.h"

namespace rj
{
    namespace net
    {
        namespace sync
        {
            extern std::shared_ptr<server_impl> create_server_impl();

            server::server(const factory_type &factory)
                : socket_server(factory), impl_(create_server_impl()), frequency_(DEFAULT_FREQUENCY)
            {
            }


            server::server(server &&other)
                : socket_server(std::move(other)), impl_(std::move(other.impl_)), frequency_(other.frequency_)
            {
                other.impl_ = nullptr;
            }

            server &server::operator=(server &&other)
            {
                socket_server::operator=(std::move(other));

                impl_ = std::move(other.impl_);

                frequency_ = other.frequency_;

                other.impl_ = nullptr;

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


            /**
             * overridden to initialize epoll
             */
            bool server::listen(const int port, const int backlogSize)
            {
                bool rval = socket_server::listen(port, backlogSize);

                if (rval && impl_) {
                    if (!impl_->listen(*this)) {
                        close();
                        return false;
                    }
                }

                return rval;
            }

            server::timer *server::wait_time(timer *last_time) const
            {
                static timer stall_time;
                timer now_time;
                long secDelta;
                long usecDelta;

                if (last_time == NULL || frequency_ <= 0) {
                    return NULL;
                }

                gettimeofday(&now_time, NULL);

                usecDelta = (last_time->tv_usec - now_time.tv_usec + 1000000) / frequency_;
                secDelta = (last_time->tv_sec - now_time.tv_sec);

                while (usecDelta < 0) {
                    usecDelta += 1000000;
                    secDelta -= 1;
                }

                while (usecDelta >= 1000000) {
                    usecDelta -= 1000000;
                    secDelta += 1;
                }

                memset(&stall_time, 0, sizeof(timer));

                gettimeofday(last_time, NULL);

                // check if server should stall for a moment based on poll frequency
                if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
                    stall_time.tv_usec = usecDelta;
                    stall_time.tv_sec = secDelta;
                }

                return &stall_time;
            }

            void server::set_frequency(unsigned value)
            {
                frequency_ = value;
            }

            void server::poll(struct timeval *last_time)
            {
                if (!is_valid()) return;

                if (impl_) {
                    impl_->poll(*this, wait_time(last_time));

                    notify_poll();
                }
            }

            void server::on_poll()
            {
            }

            void server::run()
            {
                struct timeval last_time;

                gettimeofday(&last_time, NULL);

                while (is_valid()) {
                    poll(&last_time);
                }
            }
        }
    }
}
