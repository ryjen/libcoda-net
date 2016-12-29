#ifndef RJ_NET_ASYNC_SERVER_H_
#define RJ_NET_ASYNC_SERVER_H_

#include "socket_server.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            class server : public rj::net::socket_server
            {
               public:
                /*!
                 * default constructor
                 * @factory the factory to create sockets with
                 */
                server(const factory_type &factory = default_socket_factory);

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
                std::vector<std::shared_ptr<std::thread>> threads_;
            };
        }
    }
}


#endif
