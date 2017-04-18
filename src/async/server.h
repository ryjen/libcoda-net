#ifndef RJ_NET_SERVER_ASYNC_H
#define RJ_NET_SERVER_ASYNC_H

#include "../socket_server.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            extern socket_server::factory_type socket_factory;

            class server : public rj::net::socket_server
            {
               public:
                typedef std::function<void(const socket_type &socket)> client_loop;

                /*!
                 * default constructor
                 * @factory the factory to create sockets with
                 */
                server(const factory_type &factory = socket_factory);

                /*!
                 * non-copyable constructor
                 */
                server(const server &other) = delete;

                /*!
                 * move constructor
                 */
                server(server &&other);

                virtual ~server();

                /*!
                 * Non-copyable assignment operator
                 */
                server &operator=(const server &other) = delete;

                /*!
                 * move assignment operator
                 */
                server &operator=(server &&other);

               protected:
                void run();

                void on_start();

               private:
                void set_non_blocking(bool);

                std::vector<std::shared_ptr<std::thread>> threads_;
            };
        }
    }
}


#endif
