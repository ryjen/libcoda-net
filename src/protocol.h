#ifndef ARG3_NET_PROTOCOL_H
#define ARG3_NET_PROTOCOL_H

namespace arg3
{
    namespace net
    {
        namespace http
        {

            constexpr const unsigned MAX_URL_LEN = 2000;

            constexpr const char *const SCHEME = "http";

            constexpr const char *const SECURE_SCHEME = "https";

            // default port
            constexpr static int DEFAULT_PORT = 80;

            constexpr static int DEFAULT_SECURE_PORT = 443;

            /*!
             * An http request header preamble
             */
            constexpr static const char *const REQUEST_HEADER = "%s /%s HTTP/1.0";

            /*!
             * An http response header preamble
             */
            constexpr static const char *const RESPONSE_HEADER = "HTTP / 1.1 %d %s";

            typedef enum
            {
                GET, POST, PUT, DELETE
            }
            method;

            constexpr static const char *const method_names[] = { "GET", "POST", "PUT", "DELETE" };
        }
    }
}

#endif
