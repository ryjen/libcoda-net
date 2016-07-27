#ifndef RJ_NET_PROTOCOL_H
#define RJ_NET_PROTOCOL_H

#include "socket.h"

namespace rj
{
    namespace net
    {
        namespace http
        {
            static const socket::data_buffer NEWLINE;

            constexpr static const int OK = 200;

            constexpr const unsigned MAX_URL_LEN = 2000;

            constexpr const char *const PROTOCOL = "http";

            constexpr const char *const SECURE_PROTOCOL = "https";

            // default port
            constexpr static int DEFAULT_PORT = 80;

            constexpr static int DEFAULT_SECURE_PORT = 443;

            constexpr static const char *const VERSION_1_1 = "1.1";

            constexpr static const char *const VERSION_1_0 = "1.0";

            /*!
             * An http request header preamble
             */
            constexpr static const char *const REQUEST_PREAMBLE = "%s %s HTTP/%s";

            /*!
             * An http response header preamble
             */
            constexpr static const char *const RESPONSE_PREAMBLE = "HTTP/%s %d %s";

            constexpr static int DEFAULT_HTTP_TIMEOUT = 20;

            /*!
             * header definitions
             */
            constexpr static const char *const HEADER_CONTENT_SIZE = "Content-Size";

            constexpr static const char *const HEADER_HOST = "Host";

            constexpr static const char *const HEADER_USER_AGENT = "User-Agent";

            constexpr static const char *const HEADER_CONNECTION = "Connection";

            constexpr static const char *const HEADER_ACCEPT = "Accept";

            constexpr static const char *const HEADER_TRANSFER_ENCODING = "Transfer-Encoding";

            typedef enum { GET, POST, PUT, DELETE } method;

            constexpr static const char *const method_names[] = {"GET", "POST", "PUT", "DELETE"};
        }

        namespace telnet
        {
            constexpr static const socket::data_type SE = 240;   //    End of subnegotiation parameters.
            constexpr static const socket::data_type NOP = 241;  //    No operation.
            constexpr static const socket::data_type DATA_MARK =
                242;  //    The data stream portion of a Synch.This should always be accompanied by a TCP Urgent notification.
            constexpr static const socket::data_type BREAK = 243;              //    NVT character BRK.
            constexpr static const socket::data_type INTERRUPT_PROCESS = 244;  //   The function IP.
            constexpr static const socket::data_type ABORT_OUTPUT = 245;       //    The function AO.
            constexpr static const socket::data_type ARE_YOU_THERE = 246;      //    The function AYT.
            constexpr static const socket::data_type ERASE_CHAR = 247;         //    The function EC.
            constexpr static const socket::data_type ERASE_LINE = 248;         //    The function EL.
            constexpr static const socket::data_type GO_AHEAD = 249;           //    The GA signal.
            constexpr static const socket::data_type SB = 250;  //    Indicates that what follows is subnegotiation of the indicated option.
            constexpr static const socket::data_type WILL =
                251;  //    Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
            constexpr static const socket::data_type WONT =
                252;                                            //    Indicates the refusal to perform,  or continue performing, the indicated option.
            constexpr static const socket::data_type DO = 253;  //    Indicates the request that the  other party perform, or confirmation that you
                                                                //    are expecting  the other party to perform, the indicated option.
            constexpr static const socket::data_type DONT = 254;  //    Indicates the demand that the other party stop performing, or confirmation
                                                                  //    that you are no longer expecting the other party  to perform, the indicated
                                                                  //    option.
            constexpr static const socket::data_type IAC = 255;   //    Data Byte 255.


            constexpr static const socket::data_type ECHO = 1;
            constexpr static const socket::data_type SUPPRESS_GO_AHEAD = 3;
            constexpr static const socket::data_type NAWS = 31;
            constexpr static const socket::data_type TERMINAL_TYPE = 24;
        }
    }
}

#endif
