#include "telnet_socket.h"
#include "protocol.h"
#include <algorithm>
#include <cassert>

namespace arg3
{
    namespace net
    {
        telnet_socket::telnet_socket(SOCKET sock, const sockaddr_in &addr) : buffered_socket(sock, addr)
        {}

        telnet_socket::telnet_socket(const string &host, const int port) : buffered_socket(host, port)
        {}

        socket::data_buffer::iterator telnet_socket::handle_telopt(data_buffer &s, const socket::data_buffer::iterator &it)
        {
            auto pos = (it + 1);

            if (pos == s.end()) return pos;

            on_telopt(*it, *pos);

            return s.erase(it, pos + 1);
        }

        socket::data_buffer::iterator telnet_socket::handle_sub_neg(data_buffer &s, const socket::data_buffer::iterator &it)
        {
            if ((it + 1) == s.end())
                return it;

            auto type = *(it + 1);

            auto pos = find(it + 1, s.end(), telnet::IAC);

            if (pos == s.end() || *pos != telnet::IAC)
                return it;

            if ((pos + 1) == s.end() || *(pos + 1) != telnet::SE || (it + 2) == s.end())
                return it;

            socket::data_buffer buf(it + 2, pos);

            on_sub_neg(type, buf);

            return s.erase(it, pos + 2);
        }

        void telnet_socket::on_recv(data_buffer &s)
        {
            auto pos = find(s.begin(), s.end(), telnet::IAC);

            while (pos != s.end() && (pos + 1) != s.end())
            {
                auto next = s.erase(pos);

                switch (*next)
                {
                case telnet::IAC:
                    pos = s.erase(next);
                    break;
                case telnet::WILL:
                case telnet::WONT:
                case telnet::DO:
                case telnet::DONT:
                    pos = handle_telopt(s, next);
                    break;

                case telnet::SB:
                    pos = handle_sub_neg(s, next);
                    break;
                }

                pos = find(pos, s.end(), telnet::IAC);
            }
        }

        void telnet_socket::send_telopt(socket::data_type action, socket::data_type option_value)
        {
            assert(action == telnet::WILL || action == telnet::DO
                   || action == telnet::WONT || action == telnet::DONT);

            const socket::data_type packet[] = { telnet::IAC, action, option_value };

            send( (void *) packet, 3);
        }

    }
}