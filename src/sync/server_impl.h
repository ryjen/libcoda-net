#ifndef RJ_NET_SERVER_SYNC_IMPL_H
#define RJ_NET_SERVER_SYNC_IMPL_H

#include <sys/time.h>

namespace rj
{
    namespace net
    {
        namespace sync
        {
            class server;

            class server_impl
            {
               public:
                typedef struct timeval timer;

                virtual bool listen(server &server) = 0;
                virtual void poll(server &server, timer *stall_time) = 0;
            };
        }
    }
}

#endif
