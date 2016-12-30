#ifndef RJ_NET_SOCKETFACTORY_H
#define RJ_NET_SOCKETFACTORY_H

#include "buffered_socket.h"

#include <memory>

namespace rj
{
    namespace net
    {
        class buffered_socket;
        class socket_server;

        /* factory class to control sockets in a server */
        class socket_factory
        {
           public:
            typedef socket_server *server_type;
            typedef std::shared_ptr<buffered_socket> socket_type;

            /* creates a buffered socket from a raw socket */
            virtual socket_type create_socket(const server_type &server, SOCKET sock,
                                              const struct sockaddr_storage &addr) = 0;

            virtual ~socket_factory()
            {
            }
        };

        namespace impl
        {
            /* default implementation of a socket factory */
            class default_socket_factory : public socket_factory
            {
               public:
                virtual socket_type create_socket(const server_type &server, SOCKET sock,
                                                  const struct sockaddr_storage &addr);
            };
        }

        /* default factory instance */
        extern std::shared_ptr<impl::default_socket_factory> default_socket_factory;
    }
}

#endif
