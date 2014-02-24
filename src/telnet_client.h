#ifndef ARG3_NET_TELNET_CLIENT_H_
#define ARG3_NET_TELNET_CLIENT_H_

#include "buffered_socket.h"

namespace arg3
{
    namespace net
    {
        class telnet_client : public buffered_socket
        {
        public:
            telnet_client(SOCKET sock, const sockaddr_in &addr);
        protected:
            virtual void on_telopt(socket::data_type type, socket::data_type option) = 0;
            virtual void on_sub_neg(socket::data_type type, const socket::data_buffer &parameters) = 0;
            void on_will_read();
            void on_did_read();
            void on_will_write();
            void on_did_write();
            void on_connect();
            void on_close();

            void send_telopt(socket::data_type type, socket::data_type option);
        private:
            socket::data_buffer::iterator handle_telopt(const socket::data_buffer::iterator &it);
            socket::data_buffer::iterator handle_sub_neg(const socket::data_buffer::iterator &it);
        };
    }
}

#endif
