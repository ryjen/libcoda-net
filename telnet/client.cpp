#include "client.h"
#include "protocol.h"
#include <algorithm>

namespace arg3
{
    namespace net
    {
        telnet_client::telnet_client(SOCKET sock, const sockaddr_in &addr) : buffered_socket(sock, addr)
        {}

        void telnet_client::on_telopt(socket::data_type type, socket::data_type option)
        {
            switch (type)
            {

            }
        }

        void telnet_client::on_sub_neg(socket::data_type type, const socket::data_buffer &params)
        {

        }

        void telnet_client::on_will_read()
        {}

        void telnet_client::handle_telopt(const socket::data_buffer::iterator &it)
        {
            on_telopt(*it, *(it + 1));

            inBuffer_.erase(it, it + 2);
        }

        void telnet_client::handle_sub_neg(const socket::data_buffer::iterator &it)
        {
            auto type = *(it + 1);

            auto pos = find(it + 1, inBuffer_.end(), telnet::IAC);

            if (pos == inBuffer_.end() || *pos != telnet::IAC) return;

            if ((pos + 1) == inBuffer_.end() || *(pos + 1) != telnet::SE) return;

            socket::data_buffer buf(it + 2, pos);

            on_sub_neg(type, buf);

            inBuffer_.erase(it, pos + 2);
        }

        void telnet_client::on_did_read()
        {
            auto pos = find(inBuffer_.begin(), inBuffer_.end(), telnet::IAC);

            while (pos != inBuffer_.end() && ++pos != inBuffer_.end())
            {
                switch (*pos)
                {
                case telnet::IAC:
                    pos = inBuffer_.erase(pos, pos);
                    break;
                case telnet::WILL:
                case telnet::WONT:
                case telnet::DO:
                case telnet::DONT:
                    handle_telopt(pos);
                    break;

                case telnet::SB:
                    handle_sub_neg(pos);
                }

                pos = find(pos, inBuffer_.end(), telnet::IAC);
            }
        }

        void telnet_client::on_will_write()
        {}

        void telnet_client::on_did_write()
        {}

        void telnet_client::on_connect()
        {}

        void telnet_client::on_close()
        {}

    }
}