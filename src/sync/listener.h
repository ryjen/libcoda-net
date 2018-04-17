#ifndef CODA_NET_SERVER_SYNC_LISTENER_H
#define CODA_NET_SERVER_SYNC_LISTENER_H

#include "../socket_server_listener.h"

namespace coda
{
    namespace net
    {
        namespace sync
        {
            class server;

            class server_listener : public socket_server_listener
            {
                typedef server *server_type;

               public:
                /*!
                 * called when the server has polled its connections
                 */
                virtual void on_poll(const server_type &server) = 0;
            };
        }
    }
}

#endif
