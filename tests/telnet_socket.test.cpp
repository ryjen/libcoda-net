
#include <bandit/bandit.h>
#include <string>
#include <thread>
#include "buffered_socket.h"
#include "socket_server_listener.h"
#include "sync/server.h"
#include "telnet/protocol.h"
#include "telnet/socket.h"

using namespace bandit;

using namespace rj::net;

using namespace rj;

using namespace std;

class telnet_test_client : public telnet_socket
{
   public:
    telnet_test_client(SOCKET sock, const sockaddr_storage &addr) : telnet_socket(sock, addr)
    {
    }

    telnet_test_client(const string &host, const int port) : telnet_socket(host, port)
    {
    }

    void on_telopt(socket::data_type type, socket::data_type option)
    {
        char buf[100] = {0};
        snprintf(buf, 100, "got %d %d", type, option);

        this->telopt_type = type;
        this->telopt_option = option;

        write(buf);
    };
    void on_sub_neg(socket::data_type type, const socket::data_buffer &parameters){};

    rj::net::socket::data_type telopt_type, telopt_option;
};

const socket::data_buffer will_echo{rj::net::telnet::IAC, rj::net::telnet::WILL, rj::net::telnet::ECHO};
const socket::data_buffer wont_echo{rj::net::telnet::IAC, rj::net::telnet::WONT, rj::net::telnet::ECHO};

class telnet_socket_factory : public socket_factory,
                              public buffered_socket_listener,
                              public socket_server_listener,
                              public enable_shared_from_this<telnet_socket_factory>
{
   private:
   public:
    socket_factory::socket_type create_socket(const socket_factory::server_type &server, SOCKET sock,
                                              const sockaddr_storage &addr)
    {
        auto socket = make_shared<telnet_test_client>(sock, addr);

        socket->set_non_blocking(server->is_non_blocking());

        socket->add_listener(shared_from_this());

        return socket;
    }

    void on_connect(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " connected, sending will_echo\n";

        sock->send(will_echo);
    }

    void on_close(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " closed\n";
    }

    void on_will_read(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " will read\n";
    }

    void on_did_read(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " did read\n";
    }

    void on_will_write(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " will write\n";
    }

    void on_did_write(const buffered_socket_listener::socket_type &sock)
    {
        // cout << sock->ip() << " did write\n";

        sock->close();
    }

    void on_start(const socket_server_listener::server_type &server)
    {
    }

    void on_stop(const socket_server_listener::server_type &server)
    {
    }
};

go_bandit([]() {

    std::shared_ptr<telnet_socket_factory> telnetFactory = std::make_shared<telnet_socket_factory>();

    sync::server telnetServer(telnetFactory);

    telnetServer.add_listener(telnetFactory);

    describe("a telnet socket", [&]() {
        before_each([&]() {
            try {
                telnetServer.start_in_background(8765);
            } catch (const exception &e) {
                std::cerr << e.what() << std::endl;
            }
        });

        after_each([&]() { telnetServer.stop(); });

        it("supports ECHO", []() {
            telnet_test_client client("127.0.0.1", 8765);

            Assert::That(client.is_valid(), Equals(true));

            char buf[101] = {0};
            snprintf(buf, 100, "got %d %d", rj::net::telnet::WILL, rj::net::telnet::ECHO);

            string test(buf);

            client.read_to_buffer();

            string output(client.output().begin(), client.output().end());

            Assert::That(output, Equals(test));

        });
    });


});
