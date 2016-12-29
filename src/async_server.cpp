#include "async_server.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include "exception.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            auto client_loop = [](const server::socket_type &socket) {
                while (socket->is_valid()) {
                    if (!socket->write_from_buffer()) {
                        socket->close();
                        break;
                    }

                    if (!socket->is_valid()) {
                        break;
                    }

                    if (!socket->read_to_buffer()) {
                        socket->close();
                        break;
                    }
                }
            };


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

                    auto thread = std::make_shared<std::thread>(async::client_loop, sock);

                    threads_.push_back(thread);
                }
            }
        }
    }
}
