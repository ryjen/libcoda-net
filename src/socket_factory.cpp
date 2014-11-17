

#include "socket_factory.h"

namespace arg3
{
    namespace net
    {

        detail::default_socket_factory default_socket_factory;

        namespace detail
        {
            std::shared_ptr<buffered_socket> default_socket_factory::create_socket(socket_server *server, SOCKET sock, const struct sockaddr_storage &addr)
            {
                return std::make_shared<buffered_socket>(sock, addr);
            }
        }
    }
}

