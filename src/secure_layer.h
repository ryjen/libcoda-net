
#ifndef CODA_NET_SECURE_LAYER_H
#define CODA_NET_SECURE_LAYER_H

#ifdef OPENSSL_FOUND
#include <openssl/ssl.h>
#endif
#include "socket.h"

namespace coda
{
    namespace net
    {
        class secure_layer
        {
           public:
            virtual void init() = 0;
            virtual void shutdown() = 0;
            virtual void attach(SOCKET sock) = 0;
            virtual int send(const void *data, size_t size) = 0;
            virtual int read(void *buf, size_t size) = 0;
        };

#ifdef OPENSSL_FOUND
        class openssl_layer : public secure_layer
        {
           private:
            SSL *handle_;
            SSL_CTX *context_;

           public:
            openssl_layer();
            openssl_layer(const openssl_layer &);
            openssl_layer(openssl_layer &&other);
            virtual ~openssl_layer();
            openssl_layer &operator=(const openssl_layer &);
            openssl_layer &operator=(openssl_layer &&);

            void init();
            void shutdown();
            void attach(SOCKET sock);
            int send(const void *data, size_t size);
            int read(void *buf, size_t size);
        };
#endif
    }
}
#endif