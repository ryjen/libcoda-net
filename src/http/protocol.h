#ifndef RJ_NET_HTTP_PROTOCOL_H
#define RJ_NET_HTTP_PROTOCOL_H

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

            typedef enum { OPTIONS, HEAD, GET, POST, PUT, DELETE, CONNECT, TRACE } method;

            constexpr static const char *const method_names[] = {"OPTIONS", "HEAD",   "GET",     "POST",
                                                                 "PUT",     "DELETE", "CONNECT", "TRACE"};
        }
    }
}

#endif
