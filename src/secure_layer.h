
#ifndef CODA_NET_SECURE_LAYER_H
#define CODA_NET_SECURE_LAYER_H

#ifdef OPENSSL_FOUND
#include <openssl/ssl.h>
#endif
#include "socket.h"

namespace coda {
  namespace net {
    class secure_layer {
      public:
      virtual ~secure_layer() = default;
      virtual void init() = 0;
      virtual void shutdown() = 0;
      virtual void attach(SOCKET sock) = 0;
      virtual int send(const void *data, size_t size) = 0;
      virtual int read(void *buf, size_t size) = 0;
    };

#ifdef OPENSSL_FOUND
    class openssl_layer : public secure_layer {
      private:
      SSL *handle_;
      SSL_CTX *context_;

      public:
      openssl_layer();
      openssl_layer(const openssl_layer &) = delete;
      openssl_layer(openssl_layer &&other) noexcept;
      ~openssl_layer() override;
      openssl_layer &operator=(const openssl_layer &) = delete;
      openssl_layer &operator=(openssl_layer &&other) noexcept;

      void init() override;
      void shutdown() override;
      void attach(SOCKET sock) override;
      int send(const void *data, size_t size) override;
      int read(void *buf, size_t size) override;
    };
#endif
  } // namespace net
} // namespace coda
#endif