#ifndef RJ_NET_SERVER_SYNC_LISTENER_H
#define RJ_NET_SERVER_SYNC_LISTENER_H

#include "../socket_server_listener.h"

namespace rj
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
