#ifdef EPOLL_FOUND

#include "epoll_impl.h"
#include <sys/epoll.h>
#include <cstring>
#include "../exception.h"
#include "server.h"

#define MAXEVENTS 64

namespace rj
{
    namespace net
    {
        namespace sync
        {
            impl::impl() : socket_(socket::INVALID)
            {
            }
            impl::impl(impl &&other) : socket_(std::move(other.socket_))
            {
            }
            impl::~impl()
            {
                if (socket_ != socket::INVALID) {
                    ::close(socket_);
                    socket_ = socket::INVALID;
                }
            }
            impl &impl::operator=(impl &&other)
            {
                socket_ = std::move(other.socket_);
                return *this;
            }
            bool impl::listen(server &server)
            {
                if (!server.is_valid()) {
                    return false;
                }

                struct epoll_event event;

                memset(&event, 0, sizeof(epoll_event));

                socket_ = epoll_create1(0);

                if (socket_ == socket::INVALID) {
                    return false;
                }

                event.data.fd = server.raw_socket();
                event.events = EPOLLIN | EPOLLET;

                int s = epoll_ctl(socket_, EPOLL_CTL_ADD, server.raw_socket(), &event);

                if (s == socket::INVALID) {
                    return false;
                }

                return true;
            }

            void impl::poll(server &server, struct timeval *stall_time)
            {
                if (!server.is_valid() && socket_ != socket::INVALID) {
                    return;
                }

                struct epoll_event events[MAXEVENTS] = {0};

                int n = epoll_wait(socket_, events, MAXEVENTS, stall_time == NULL ? -1 : stall_time->tv_usec / 1000);

                if (n == socket::INVALID) {
                    throw socket_exception(strerror(errno));
                }

                for (int i = 0; i < n; i++) {
                    if (events[i].events & EPOLLERR) {
                        auto socket = server.find_socket(events[i].data.fd);

                        if (socket != nullptr) {
                            socket->close();
                        }
                        continue;
                    }

                    if (events[i].events & EPOLLIN) {
                        if (server.raw_socket() == events[i].data.fd) {
                            sockaddr_storage addr;

                            int infd = server.accept(addr);
                            if (infd != socket::INVALID) {
                                server.on_accept(infd, addr);
                            }
                            continue;
                        }

                        auto c = server.find_socket(events[i].data.fd);

                        if (c != nullptr) {
                            if (!c->read_to_buffer()) {
                                c->close();
                                continue;
                            }
                        }
                    }

                    if (events[i].events & EPOLLOUT) {
                        auto c = server.find_socket(events[i].data.fd);
                        if (c != nullptr && c->has_output()) {
                            if (!c->write_from_buffer()) {
                                c->close();
                                continue;
                            }
                        }
                    }
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
