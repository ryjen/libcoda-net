#ifndef CODA_NET_SERVER_SYNC_EPOLL_IMPL_H
#define CODA_NET_SERVER_SYNC_EPOLL_IMPL_H

#ifdef EPOLL_FOUND

#include "../socket.h"
#include "server_impl.h"

namespace coda {
  namespace net {
    namespace sync {
      class impl : public server_impl {
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
        SOCKET socket_;
      };
    } // namespace sync
  }   // namespace net
} // namespace coda

#endif

#endif
