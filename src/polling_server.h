#ifndef RJ_NET_POLLING_SERVER_H
#define RJ_NET_POLLING_SERVER_H

#include "socket_server.h"

namespace rj
{
    namespace net
    {
        namespace polling
        {
            class server;

            class server_listener : public socket_server_listener
            {
                typedef server *server_type;

               public:
                /*!
                 * called when the server has polled its connections
                 */
                virtual void on_poll(const server_type &server) = 0;
            };

            class server : public socket_server
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

                /*!
                 * Non-copyable assignment operator
                 */
                server &operator=(const server &other) = delete;

                /*!
                 * move assignment operator
                 */
                server &operator=(server &&other);

                /*!
                 * Sets the frequency of connection updates (used when looping)
                 * Only uses in non-async
                 * @param value the number of cycles per secon
                 */
                void set_frequency(unsigned cyclesPerSecond);

                /*!
                 * updates the servers connections (performs read/writes)
                 * - will do nothing if the server is not alive
                 * - is called by the server based on the poll frequency
                 */
                void poll();

               protected:
                void run();

                virtual void on_start();

                virtual void on_poll();

               private:
                constexpr static int DEFAULT_POLL_FREQUENCY = 4;

                /*!
                 * Will loop each connection and if the delegate returns false, will remove the connection
                 */
                void check_connections(std::function<bool(const socket_type &)> delegate);

                void wait_for_poll(struct timeval *);

                void notify_poll();

                unsigned frequency_;
            };
        }
    }
}

#endif
