#include "server.h"
#include "../exception.h"
#include "client.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>

namespace coda {
  namespace net {
    namespace async {
      server::server(const factory_type &factory) : socket_server(factory) {}

      server::server(server &&other) : socket_server(std::move(other)) {}

      server::~server() {}

      server &server::operator=(server &&other) {
        server::operator=(std::move(other));
        return *this;
      }

      void server::on_start() { socket_server::set_non_blocking(false); }

      void server::run() {
        while (is_valid()) {
          sockaddr_storage addr;

          auto sys_sock = accept(addr);

          if (sys_sock == INVALID) {
            continue;
          }

          on_accept(sys_sock, addr);
        }
      }
    } // namespace async
  }   // namespace net
} // namespace coda
