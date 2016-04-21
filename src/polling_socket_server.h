#ifndef ARG3_NET_POLLING_SOCKET_SERVER_H_
#define ARG3_NET_POLLING_SOCKET_SERVER_H_

#include "socket_server.h"

namespace arg3
{
    namespace net
    {
        class polling_socket_server : public socket_server
        {
        public:

            /*!
             * default constructor
             * @factory the factory to create sockets with
             */
            polling_socket_server(socket_factory *factory = &default_socket_factory);

            /*!
             * non-copyable constructor
             */
            polling_socket_server(const polling_socket_server &other) = delete;

            /*!
             * move constructor
             */
            polling_socket_server(polling_socket_server &&other);

            /*!
             * Non-copyable assignment operator
             */
            polling_socket_server &operator=(const polling_socket_server &other) = delete;

            /*!
             * move assignment operator
             */
            polling_socket_server &operator=(polling_socket_server &&other);

            /*!
             * Sets the frequency of connection updates (used when looping)
             * Only uses in non-async
             * @param value the number of cycles per secon
             */
            void set_poll_frequency(unsigned cyclesPerSecond);

            /*!
             * updates the servers connections (performs read/writes)
             * - will do nothing if the server is not alive
             * - is called by the server based on the poll frequency
             */
            void poll();

        protected:

            void run();

            virtual void on_start();

        private:
            constexpr static int DEFAULT_POLL_FREQUENCY = 4;

            /*!
             * Will loop each connection and if the delegate returns false, will remove the connection
             */
            void check_connections(std::function<bool(const std::shared_ptr<buffered_socket> &)> delegate);

            void wait_for_poll(struct timeval *);

            void notify_poll();

            unsigned pollFrequency_;

        };
    }
}


#endif
