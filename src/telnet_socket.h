#ifndef ARG3_NET_TELNET_SOCKET_H_
#define ARG3_NET_TELNET_SOCKET_H_

#include "buffered_socket.h"

namespace arg3
{
    namespace net
    {
        class telnet_socket : public buffered_socket
        {
           public:
            telnet_socket(SOCKET sock, const sockaddr_storage &addr);
            telnet_socket(const std::string &host, const int port);

           protected:
            virtual void on_telopt(socket::data_type type, socket::data_type option) = 0;
            virtual void on_sub_neg(socket::data_type type, const socket::data_buffer &parameters) = 0;

            void on_recv(data_buffer &s);

            void send_telopt(socket::data_type type, socket::data_type option);

           private:
            socket::data_buffer::iterator handle_telopt(data_buffer &s, const socket::data_buffer::iterator &it);
            socket::data_buffer::iterator handle_sub_neg(data_buffer &s, const socket::data_buffer::iterator &it);
        };
    }
}

#endif
