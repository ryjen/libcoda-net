#ifndef ARG3_NET_PROTOCOL_H
#define ARG3_NET_PROTOCOL_H

namespace arg3
{
    namespace net
    {
        namespace http
        {

            extern const char *const DELETE_METHOD;

            extern const unsigned MAX_URL_LEN;

            extern const char *const SCHEME;

            extern const char *const SECURE_SCHEME;

            constexpr static int HTTP_PORT = 80;

            constexpr static const char *const HTTP_REQUEST = "%s /%s HTTP/1.0";

            constexpr static const char *const HTTP_RESPONSE = "HTTP / 1.1 %d %s";

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
