#ifndef ARG3_NET_SOCKET_SERVER_H
#define ARG3_NET_SOCKET_SERVER_H

#include <sys/time.h>
#include <mutex>
#include <thread>
#include <vector>
#include "socket_factory.h"

using namespace std;

namespace arg3
{
    namespace net
    {
        class socket_server;

        /*!
         * Defines an interface to listen to a socket server
         */
        class socket_server_listener
        {
           public:
            typedef socket_server *server_type;

            /*!
             * called when the server starts
             */
            virtual void on_start(const server_type &server) = 0;

            /*!
             * called when the server stops
             */
            virtual void on_stop(const server_type &server) = 0;
        };


        /*!
         * Defines a network server that accepts incomming connections, processes their input and writes their output.
         */
        class socket_server : public socket
        {
           public:
            typedef std::shared_ptr<socket_server_listener> listener_type;
            typedef std::shared_ptr<buffered_socket> socket_type;
            typedef std::shared_ptr<socket_factory> factory_type;

            /*!
             * default constructor
             * @factory the factory to create sockets with
             */
            socket_server(const factory_type &factory = default_socket_factory);

            /*!
             * non-copyable constructor
             */
            socket_server(const socket_server &other) = delete;

            /*!
             * move constructor
             */
            socket_server(socket_server &&other);

            /*!
             * destuctor will stop the server
             */
            virtual ~socket_server();

            /*!
             * Non-copyable assignment operator
             */
            socket_server &operator=(const socket_server &other) = delete;

            /*!
             * move assignment operator
             */
            socket_server &operator=(socket_server &&other);

            bool is_valid() const;

            /*!
             * starts the server
             */
            void start(int port, int backlogSize = BACKLOG_SIZE);

            void start_in_background(int port, int backlogSize = BACKLOG_SIZE);

            /*!
             * Adds a l istener to the server
             */
            socket_server &add_listener(const listener_type &listener);

            /*!
             * equality operator compares the port
             */
            bool operator==(const socket_server &other);

            /*!
             * inequality operator compares the port
             */
            bool operator!=(const socket_server &other);

            void stop();

            bool listen(const int port, const int backlogSize);

            void set_socket_factory(const factory_type &factory);

           protected:
            /*!
             * starts a syncronous loop of updating connections
             */
            virtual void run();

            /*!
             * can override these without adding a listener
             */
            virtual void on_poll();
            virtual void on_start();
            virtual void on_stop();

            mutex sockets_mutex_;
            mutex listeners_mutex_;
            vector<socket_type> sockets_;
            vector<listener_type> listeners_;
            factory_type factory_;

           private:
            void notify_start();

            void notify_stop();

            shared_ptr<thread> backgroundThread_;
        };
    }
}

#endif
