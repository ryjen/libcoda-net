
#include "../socket_server.h"
#include "client.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            default_client::default_client(SOCKET sock, const sockaddr_storage &addr) : buffered_socket(sock, addr)
            {
            }

            default_client::default_client(const std::string &host, const int port) : buffered_socket(host, port)
            {
            }

            default_client::default_client()
            {
            }


            /*!
             * Move constructor
             */
            default_client::default_client(default_client &&other)
                : buffered_socket(std::move(other)), backgroundThread_(std::move(other.backgroundThread_))
            {
            }

            /*!
             * Destructor
             */
            default_client::~default_client()
            {
                if (backgroundThread_ && backgroundThread_->joinable()) {
                    backgroundThread_->join();
                }
            }

            void default_client::on_connect()
            {
                backgroundThread_ = std::make_shared<std::thread>(&default_client::run, this);
            }

            /*!
             * Move assigment
             */
            default_client &default_client::operator=(default_client &&other)
            {
                buffered_socket::operator=(std::move(other));

                backgroundThread_ = std::move(other.backgroundThread_);

                return *this;
            }

            void default_client::run()
            {
                while (is_valid()) {
                    if (!write_from_buffer()) {
                        close();
                        break;
                    }

                    if (!is_valid()) {
                        break;
                    }

                    if (!read_to_buffer()) {
                        close();
                        break;
                    }
                }
            }
        }
    }
}
