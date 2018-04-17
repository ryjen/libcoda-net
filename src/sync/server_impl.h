#ifndef CODA_NET_SERVER_SYNC_IMPL_H
#define CODA_NET_SERVER_SYNC_IMPL_H

#include <sys/time.h>

namespace coda
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
