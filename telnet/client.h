#ifndef ARG3_NET_TELNET_CLIENT_H_
#define ARG3_NET_TELNET_CLIENT_H_

#include "../bufferedsocket.h"

namespace arg3
{
    namespace net
    {
        class telnet_client : public buffered_socket
        {
        public:
            telnet_client(SOCKET sock, const sockaddr_in &addr);
        protected:
            void on_telopt(socket::data_type type, socket::data_type option);
            void on_sub_neg(socket::data_type type, const socket::data_buffer &parameters);
            void on_will_read();
            void on_did_read();
            void on_will_write();
            void on_did_write();
            void on_connect();
            void on_close();
        private:
            void handle_telopt(const socket::data_buffer::iterator &it);
            void handle_sub_neg(const socket::data_buffer::iterator &it);
        };
    }
}

#endif
