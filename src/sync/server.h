#ifndef CODA_NET_SERVER_SYNC_H
#define CODA_NET_SERVER_SYNC_H

#include "../socket_server.h"
#include "server_impl.h"

namespace coda {
  namespace net {
    namespace sync {
      namespace detail {
        class cleanup_listener;
      }

      /*!
       * A syncronous server has a poll frequency and no threads
       */
      class server : public socket_server {
        public:
        typedef struct timeval timer;

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
         * updates the servers connections (performs read/writes)
         * - will do nothing if the server is not alive
         * - is called by the server based on the poll frequency
         */
        void poll(timer *last_time);

        bool listen(const int port, const int backlogSize);

        /*!
         * Sets the frequency of connection updates (used when looping)
         * Only uses in non-async
         * @param value the number of cycles per secon
         */
        void set_frequency(unsigned cyclesPerSecond);

        void stop();

        protected:
        void add_socket(const socket_type &sock);

        void remove_socket(const SOCKET &sock);

        void clear_sockets();

        virtual socket_type on_accept(SOCKET socket, sockaddr_storage addr);

        timer *wait_time(timer *last_time) const;

        private:
        static const unsigned DEFAULT_FREQUENCY = 4;

        std::recursive_mutex sockets_mutex_;
        socket_type find_socket(SOCKET value) const;

        std::shared_ptr<server_impl> impl_;

        void run();

        virtual void on_start();

        virtual void on_poll();

        void notify_poll();

        std::map<SOCKET, socket_type> sockets_;

        unsigned frequency_;

        friend class impl;
        friend class detail::cleanup_listener;
      };
    } // namespace sync
  }   // namespace net
} // namespace coda

#endif
