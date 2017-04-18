#ifndef RJ_NET_SOCKET_SERVER_H
#define RJ_NET_SOCKET_SERVER_H

#include <sys/time.h>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include "socket_factory.h"

namespace rj
{
    namespace net
    {
        class socket_server_listener;

        namespace detail
        {
            class cleanup_listener;
        }

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

            /*!
             * starts the server in a background thread
             */
            void start_in_background(int port, int backlogSize = BACKLOG_SIZE);

            /*!
             * Adds a listener to the server
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

            /*!
             * stops the server
             */
            void stop();

            /*!
             * listens on a port
             */
            bool listen(const int port, const int backlogSize);

            /*!
             * sets the factory used to create sockets on connections
             */
            void set_socket_factory(const factory_type &factory);

           protected:
            /*!
             * executes a server loop
             */
            virtual void run();

            /*!
             * call when the server loop accepts a new connection
             */
            virtual socket_type on_accept(SOCKET socket, sockaddr_storage addr);

            /*!
             * can override these without adding a listener
             */
            virtual void on_start();
            virtual void on_stop();

            void on_close(const buffered_socket_listener::socket_type &sock);

            std::recursive_mutex sockets_mutex_;
            std::recursive_mutex listeners_mutex_;
            std::map<SOCKET, socket_type> sockets_;
            std::vector<listener_type> listeners_;
            factory_type factory_;

            socket_type find_socket(SOCKET value) const;

           private:
            friend class detail::cleanup_listener;

            void add_socket(const socket_type &sock);

            void remove_socket(const SOCKET &sock);

            void clear_sockets();

            void notify_start();

            void notify_stop();

            std::shared_ptr<std::thread> backgroundThread_;
        };
    }
}

#endif
