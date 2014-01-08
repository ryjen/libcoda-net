
#include "factory.h"
#include "client.h"

namespace arg3
{
    namespace net
    {
        std::shared_ptr<buffered_socket> telnet_factory::create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr)
        {
            return std::make_shared<telnet_client>(sock, addr);
        }

    }
}