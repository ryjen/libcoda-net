#include "config.h"
#undef VERSION
#include <bandit/bandit.h>
#include "telnet_socket.h"
#include "socket_server.h"
#include "buffered_socket.h"
#include "protocol.h"
#include <string>
#include <thread>

using namespace bandit;

using namespace arg3::net;

using namespace arg3;

using namespace std;


class telnet_test_client : public telnet_socket
{

public:
    telnet_test_client(SOCKET sock, const sockaddr_in &addr) : telnet_socket(sock, addr)
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
    void on_sub_neg(socket::data_type type, const socket::data_buffer &parameters)
    {
    };

    arg3::net::socket::data_type telopt_type, telopt_option;

};

const socket::data_buffer will_echo
{
    arg3::net::telnet::IAC, arg3::net::telnet::WILL, arg3::net::telnet::ECHO
};
const socket::data_buffer wont_echo
{
    arg3::net::telnet::IAC, arg3::net::telnet::WONT, arg3::net::telnet::ECHO
};

class telnet_socket_factory : public socket_factory, public buffered_socket_listener, public socket_server_listener
{

private:
public:
    std::shared_ptr<buffered_socket> create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr)
    {
        auto socket = make_shared<telnet_test_client>(sock, addr);

        socket->add_listener(this);

        return socket;
    }

    void on_connect(buffered_socket *sock)
    {
        //cout <<  sock->ip() << " connected, sending will_echo\n";

        sock->send(will_echo);
    }

    void on_close(buffered_socket *sock)
    {
        //cout << sock->ip() << " closed\n";
    }

    void on_will_read(buffered_socket *sock)
    {
        //cout << sock->ip() << " will read\n";

    }

    void on_did_read(buffered_socket *sock)
    {
        //cout << sock->ip() << " did read\n";
    }

    void on_will_write(buffered_socket *sock)
    {
        //cout << sock->ip() << " will write\n";
    }

    void on_did_write(buffered_socket *sock)
    {
        //cout << sock->ip() << " did write\n";

        sock->close();
    }

    void on_poll(socket_server *server)
    {
    }

    void on_start(socket_server *server)
    {
    }

    void on_stop(socket_server *server)
    {
    }
};

go_bandit([]()
{

    telnet_socket_factory telnetFactory;

    socket_server telnetServer(9876, &telnetFactory);

    telnetServer.add_listener(&telnetFactory);

    describe("a telnet socket", [&]()
    {
        before_each([&]()
        {
            try
            {
                telnetServer.start_in_background();

                //log::trace("Mock server started");
            }
            catch (const exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        });

        after_each([&]()
        {
            telnetServer.stop();
        });

        it("supports ECHO", []()
        {
            telnet_test_client client("127.0.0.1", 9876);

            client.set_non_blocking(false);

            sleep(1);

            Assert::That(client.is_valid(), Equals(true));

            char buf[101] = {0};
            snprintf(buf, 100, "got %d %d", arg3::net::telnet::WILL, arg3::net::telnet::ECHO);

            string test(buf);

            arg3::net::socket::data_buffer data;

            client.recv(data);

            string output(client.output().begin(), client.output().end());

            Assert::That(output, Equals(test));

        });
    });


});

