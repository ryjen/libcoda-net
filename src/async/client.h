#ifndef RJ_NET_SERVER_ASYNC_CLIENT_H
#define RJ_NET_SERVER_ASYNC_CLIENT_H

namespace rj
{
    namespace net
    {
        namespace async
        {
            class client
            {
               public:
                virtual void run() = 0;
            };

            class default_client : public buffered_socket, public client
            {
               public:
                default_client(SOCKET sock, const sockaddr_storage &addr);

                default_client(const std::string &host, const int port);

                default_client();

                /*!
                 * Non copyable
                 */
                default_client(const default_client &) = delete;

                /*!
                 * Move constructor
                 */
                default_client(default_client &&other);

                /*!
                 * Destructor
                 */
                virtual ~default_client();

                /*!
                 * Non copy-assignable
                 */
                default_client &operator=(const default_client &other) = delete;

                /*!
                 * Move assigment
                 */
                default_client &operator=(default_client &&other);

                void run();

               protected:
                void on_connect();

               private:
                std::shared_ptr<std::thread> backgroundThread_;
            };
        }
    }
}

#endif
