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

            typedef enum
            {
                GET, POST, PUT, DELETE
            }
            method;
        }
    }
}

#endif
