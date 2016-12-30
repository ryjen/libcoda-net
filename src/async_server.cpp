#include "async_server.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include "exception.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
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

            namespace impl
            {
                class socket_factory : public net::socket_factory
                {
                   public:
                    virtual socket_type create_socket(const server_type &server, SOCKET sock,
                                                      const struct sockaddr_storage &addr)
                    {
                        return std::make_shared<default_client>(sock, addr);
                    }
                };
            }

            socket_server::factory_type socket_factory = std::make_shared<impl::socket_factory>();

            server::server(const factory_type &factory) : socket_server(factory)
            {
            }


            server::server(server &&other) : socket_server(std::move(other))
            {
            }

            server::~server()
            {
                for (auto &thread : threads_) {
                    if (thread && thread->joinable()) {
                        thread->join();
                    }
                }
            }

            server &server::operator=(server &&other)
            {
                server::operator=(std::move(other));
                return *this;
            }

            void server::on_start()
            {
                set_non_blocking(false);
            }

            void server::run()
            {
                struct timeval last_time;

                gettimeofday(&last_time, NULL);

                while (is_valid()) {
                    sockaddr_storage addr;

                    auto sys_sock = accept(addr);

                    if (sys_sock == INVALID) {
                        continue;
                    }

                    auto sock = on_accept(sys_sock, addr);

                    auto client = std::dynamic_pointer_cast<async::client>(sock);

                    // if async::client is implemented, run it in a thread
                    // otherwise we assume the factory took care of the thread itself
                    if (client) {
                        auto thread = std::make_shared<std::thread>(&client::run, client);

                        threads_.push_back(thread);
                    }
                }
            }
        }
    }
}
