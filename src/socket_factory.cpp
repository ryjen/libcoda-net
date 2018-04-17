

#include "socket_factory.h"
#include "socket_server.h"

namespace coda
{
    namespace net
    {
        std::shared_ptr<impl::default_socket_factory> default_socket_factory =
            std::make_shared<impl::default_socket_factory>();

        namespace impl
        {
            socket_factory::socket_type default_socket_factory::create_socket(const server_type &server, SOCKET sock,
                                                                              const struct sockaddr_storage &addr)
            {
                auto socket = std::make_shared<buffered_socket>(sock, addr);

                socket->set_non_blocking(server->is_non_blocking());

                return socket;
            }
        }
    }
}
