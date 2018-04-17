
#ifndef EPOLL_FOUND

#include "select_impl.h"
#include <sys/select.h>
#include "../exception.h"

namespace coda
{
    namespace net
    {
        namespace sync
        {
            impl::impl() : maxdesc_(0)
            {
                FD_ZERO(&in_set_);
                FD_ZERO(&out_set_);
            }

            impl::impl(impl &&other)
                : maxdesc_(std::move(other.maxdesc_)),
                  in_set_(std::move(other.in_set_)),
                  out_set_(std::move(other.out_set_))
            {
            }
            impl::~impl()
            {
                maxdesc_ = 0;
                FD_ZERO(&in_set_);
                FD_ZERO(&out_set_);
            }

            impl &impl::operator=(impl &&other)
            {
                maxdesc_ = std::move(other.maxdesc_);
                in_set_ = std::move(other.in_set_);
                out_set_ = std::move(other.out_set_);
                return *this;
            }

            bool impl::listen(server &server)
            {
                maxdesc_ = server.raw_socket();
                FD_SET(maxdesc_, &in_set_);
                return true;
            }

            void impl::poll(server &server, struct timeval *stall_time)
            {
                if (!server.is_valid()) {
                    return;
                }

                auto server_socket = server.raw_socket();

                // poll
                if (select(maxdesc_ + 1, &in_set_, &out_set_, NULL, stall_time) < 0) {
                    if (errno != EINTR) {
                        throw socket_exception(strerror(errno));
                    }
                }

                // check for new connection
                if (FD_ISSET(server_socket, &in_set_)) {
                    sockaddr_storage addr;

                    auto sock = server.accept(addr);

                    if (sock != socket::INVALID) {
                        server.on_accept(sock, addr);

                        maxdesc_ = std::max(maxdesc_, sock);
                        FD_SET(sock, &in_set_);
                        FD_SET(sock, &out_set_);
                    }
                }


                /**
                 * read/write from all connections, removing failed sockets
                 */
                iterate_connections(server, [&](const server::socket_type &c) {
                    if (!c || !c->is_valid()) {
                        return true;
                    }

                    if (FD_ISSET(c->raw_socket(), &in_set_)) {
                        if (!c->read_to_buffer()) {
                            return true;
                        }
                    }

                    if (FD_ISSET(c->raw_socket(), &out_set_)) {
                        if (c->has_output()) {
                            if (!c->write_from_buffer()) {
                                return true;
                            }
                        }
                    }
                    return false;
                });
            }

            void impl::clear(const SOCKET &c)
            {
                if (c == socket::INVALID) {
                    return;
                }
                FD_CLR(c, &in_set_);
                FD_CLR(c, &out_set_);
            }

            void impl::iterate_connections(server &server, std::function<bool(const server::socket_type &)> delegate)
            {
                if (!server.is_valid() || delegate == nullptr) {
                    return;
                }

                std::lock_guard<std::recursive_mutex> lock(server.sockets_mutex_);

                auto it = server.sockets_.begin();

                while (it != server.sockets_.end()) {
                    // test delegate
                    if (!delegate(it->second)) {
                        ++it;
                        continue;
                    }

                    // clear the polling
                    clear(it->first);

                    // remove from list
                    it = server.sockets_.erase(it);
                }
            }


            std::shared_ptr<server_impl> create_server_impl()
            {
                return std::make_shared<impl>();
            }
        }
    }
}

#endif
