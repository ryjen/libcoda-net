#ifndef CODA_NET_SOCKET_SERVER_LISTENER_H
#define CODA_NET_SOCKET_SERVER_LISTENER_H

namespace coda {
  namespace net {
    class socket_server;

    /*!
     * Defines an interface to listen to a socket server
     */
    class socket_server_listener {
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
  } // namespace net
} // namespace coda

#endif
