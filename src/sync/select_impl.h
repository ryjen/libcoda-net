#ifndef RJ_NET_SERVER_SYNC_SELECT_IMPL_H
#define RJ_NET_SERVER_SYNC_SELECT_IMPL_H

#ifndef EPOLL_FOUND

#include <functional>
#include "server.h"
#include "server_impl.h"

namespace rj
{
    namespace net
    {
        namespace sync
        {
            class impl : public server_impl
            {
               public:
                impl();
                impl(const impl &other) = delete;
                impl(impl &&other);
                virtual ~impl();
                impl &operator=(const impl &other) = delete;
                impl &operator=(impl &&other);

                bool listen(server &server);

                void poll(server &server, struct timeval *stall_time);

               private:
                void clear(const SOCKET &c);

                void iterate_connections(server &server, std::function<bool(const server::socket_type &)> delegate);

                fd_set in_set_;
                fd_set out_set_;
                int maxdesc_;
            };
        }
    }
}
#endif

#endif
