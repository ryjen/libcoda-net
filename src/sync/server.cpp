#include "server.h"
#include <algorithm>
#include <cassert>
#include <cstring>

#include "../exception.h"
#include "listener.h"

namespace coda
{
    namespace net
    {
        namespace sync
        {
            namespace detail
            {
                class cleanup_listener : public buffered_socket_listener
                {
                   private:
                    server &server_;

                   public:
                    cleanup_listener(server &server) : server_(server)
                    {
                    }

                    void on_close(const buffered_socket_listener::socket_type &socket)
                    {
                        if (!socket || !socket->is_valid()) {
                            return;
                        }

                        server_.remove_socket(socket->raw_socket());
                    }


                    void on_connect(const buffered_socket_listener::socket_type &sock)
                    {
                    }

                    void on_will_read(const buffered_socket_listener::socket_type &sock)
                    {
                    }

                    void on_did_read(const buffered_socket_listener::socket_type &sock)
                    {
                    }

                    void on_will_write(const buffered_socket_listener::socket_type &sock)
                    {
                    }

                    void on_did_write(const buffered_socket_listener::socket_type &sock)
                    {
                    }
                };
            }

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

            void server::stop()
            {
                socket_server::stop();

                clear_sockets();
            }

            server::socket_type server::on_accept(SOCKET sock, sockaddr_storage addr)
            {
                auto socket = socket_server::on_accept(sock, addr);

                add_socket(socket);

                return socket;
            }

            socket_server::socket_type server::find_socket(SOCKET value) const
            {
                auto it = sockets_.find(value);

                if (it == sockets_.end()) {
                    return nullptr;
                }

                return it->second;
            }


            void server::add_socket(const socket_type &sock)
            {
                if (!sock || !sock->is_valid()) {
                    return;
                }

                // TODO: recursive mutex could get heavy
                std::lock_guard<std::recursive_mutex> lock(sockets_mutex_);

                sock->add_listener(std::make_shared<detail::cleanup_listener>(*this));

                sockets_[sock->raw_socket()] = sock;
            }

            void server::remove_socket(const SOCKET &sock)
            {
                // TODO: recursive mutex could get heavy
                std::lock_guard<std::recursive_mutex> lock(sockets_mutex_);

                sockets_.erase(sock);
            }

            void server::clear_sockets()
            {
                // TODO: recursive mutex could get heavy
                std::lock_guard<std::recursive_mutex> lock(sockets_mutex_);
                sockets_.clear();
            }
        }
    }
}
