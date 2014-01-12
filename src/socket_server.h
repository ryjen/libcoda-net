#ifndef ARG3_NET_SOCKETSERVER_H
#define ARG3_NET_SOCKETSERVER_H

#include "socket_factory.h"
#include <vector>
#include <thread>
#include <sys/time.h>

using namespace std;

namespace arg3
{
    namespace net
    {
        class socket_server;

        class socket_server_listener
        {
        public:
            virtual void on_poll(socket_server *server) = 0;

            virtual void on_start(socket_server *server) = 0;

            virtual void on_stop(socket_server *server) = 0;
        };

        class socket_server : public socket
        {
        public:
            socket_server(int port, socket_factory *factory = &default_socket_factory, int backlogSize = BACKLOG_SIZE);
            socket_server(const socket_server &other) = delete;
            socket_server(socket_server &&other);
            virtual ~socket_server();
            socket_server &operator=(const socket_server &other) = delete;
            socket_server &operator=(socket_server && other);

            void start();

            void update();

            void loop();

            void stop();

            void set_poll_frequency(unsigned value);

            void add_listener(socket_server_listener *listener);

            bool operator==(const socket_server &other);
            bool operator!=(const socket_server &other);

        protected:

            virtual void on_poll();

            virtual void on_start();

            virtual void on_stop();

        private:

            void foreach(std::function<bool(std::shared_ptr<buffered_socket> )> delegate);

            void notify_poll();

            void notify_start();

            void notify_stop();

            unsigned pollFrequency_;

            thread listenThread_;

            socket_factory *factory_;

            vector<socket_server_listener *> listeners_;

            vector<std::shared_ptr<buffered_socket>> sockets_;
        };
    }
}

#endif

