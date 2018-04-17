#include "secure_layer.h"
#include "exception.h"

#ifdef OPENSSL_FOUND
#include <openssl/err.h>
#include <openssl/rand.h>
#endif

namespace coda
{
    namespace net
    {
#ifdef OPENSSL_FOUND
        openssl_layer::openssl_layer() : handle_(NULL), context_(NULL)
        {
            init();
        }
        openssl_layer::openssl_layer(const openssl_layer &other) : handle_(other.handle_), context_(other.context_)
        {
        }
        openssl_layer::openssl_layer(openssl_layer &&other) : handle_(other.handle_), context_(other.context_)
        {
            other.handle_ = NULL;
            other.context_ = NULL;
        }
        openssl_layer::~openssl_layer()
        {
            shutdown();
        }
        openssl_layer &openssl_layer::operator=(const openssl_layer &other)
        {
            handle_ = other.handle_;
            context_ = other.context_;
            return *this;
        }
        openssl_layer &openssl_layer::operator=(openssl_layer &&other)
        {
            handle_ = other.handle_;
            context_ = other.context_;
            other.handle_ = NULL;
            other.context_ = NULL;
            return *this;
        }

        void openssl_layer::init()
        {
            static bool initialized = false;

            if (!initialized) {
                // Register the available ciphers and digests
                SSL_library_init();
                // Register the error strings for libcrypto & libssl
                SSL_load_error_strings();

                initialized = true;
            }

            if (context_ == NULL) {
                // New context saying we are a client, and using SSL 2 or 3
                context_ = SSL_CTX_new(SSLv23_client_method());
                if (context_ == NULL) {
                    throw socket_exception(ERR_error_string(ERR_get_error(), NULL));
                }
            }

            if (handle_ == NULL) {
                // Create an SSL struct for the connection
                handle_ = SSL_new(context_);

                if (handle_ == NULL) {
                    throw socket_exception(ERR_error_string(ERR_get_error(), NULL));
                }
            }
        }

        void openssl_layer::shutdown()
        {
            if (handle_ != NULL) {
                SSL_shutdown(handle_);
                SSL_free(handle_);
                handle_ = NULL;
            }
            if (context_ != NULL) {
                SSL_CTX_free(context_);
                context_ = NULL;
            }
        }

        void openssl_layer::attach(SOCKET sock)
        {
            if (sock == socket::INVALID || handle_ == NULL) {
                return;
            }

            // Connect the SSL struct to our connection
            if (!SSL_set_fd(handle_, sock)) {
                throw socket_exception(ERR_error_string(ERR_get_error(), NULL));
            }

            // Initiate SSL handshake
            if (SSL_connect(handle_) != 1) {
                throw socket_exception(ERR_error_string(ERR_get_error(), NULL));
            }
        }

        int openssl_layer::send(const void *data, size_t size)
        {
            if (handle_ == NULL) {
                return 0;
            }
            return SSL_write(handle_, data, size);
        }

        int openssl_layer::read(void *buf, size_t size)
        {
            if (handle_ == NULL) {
                return 0;
            }
            return SSL_read(handle_, buf, size);
        }
#endif
    }
}
