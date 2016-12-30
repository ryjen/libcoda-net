#include <bandit/bandit.h>
#include <string>
#include <thread>
#include "async_server.h"
#include "http/client.h"

using namespace bandit;

using namespace rj::net;

using namespace rj;

using namespace std;

namespace test
{
    class socket_factory : public net::socket_factory,
                           public buffered_socket_listener,
                           public enable_shared_from_this<test::socket_factory>
    {
       private:
        string response_;

       public:
        net::socket_factory::socket_type create_socket(const server_type &server, SOCKET sock,
                                                       const sockaddr_storage &addr)
        {
            auto socket = std::make_shared<async::default_client>(sock, addr);

            socket->set_non_blocking(server->is_non_blocking());

            socket->add_listener(shared_from_this());

            return socket;
        }

        void set_response(const string &response)
        {
            response_ = response;
        }

        void on_connect(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " connected" << endl;
        }

        void on_close(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " closed" << endl;
        }

        void on_will_read(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " will read" << endl;
        }

        void on_did_read(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " did read" << endl;

            string line = sock->readln();

            string method = line.substr(0, line.find(' '));

            sock->write(method + ": " + response_);
        }

        void on_will_write(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " will write" << endl;
        }

        void on_did_write(const buffered_socket_listener::socket_type &sock)
        {
            // cout << sock->ip() << " did write" << endl;

            sock->close();
        }
    };
}

go_bandit([]() {

    auto testFactory = std::make_shared<test::socket_factory>();

    async::server testServer(testFactory);

    describe("an http client", [&]() {
        before_each([&testServer, &testFactory]() {
            try {
                testServer.start_in_background(9876);

                testFactory->set_response("Hello, World!");

            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
            }
        });

        after_each([&testServer]() { testServer.stop(); });

        it("can get", [&]() {
            http::client client("localhost:9876/test");

            client.get(
                [](const http::response &response) { Assert::That(response.content(), Equals("GET: Hello, World!")); });
        });
#ifdef OPENSSL_FOUND
        it("is secure", []() {
            http::client client("https://ryan-jennings.net");

            Assert::That(client.is_secure(), IsTrue());

            client.get();

            Assert::That(client.response().content().empty(), Equals(false));
        });
#endif
        it("can post", []() {
            http::client client("localhost:9876/test");

            client.set_content("Hello, World!");

            client.post();

            Assert::That(client.response().content(), Equals("POST: Hello, World!"));

        });

        it("can read http response", []() {
            http::client client("ryan-jennings.net");

            try {
                client.get();

                auto response = client.response();

                Assert::That(response.content().empty(), Equals(false));

                Assert::That(response.content().find("<html>"), !Equals(string::npos));

            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
                throw e;
            }
        });
    });

});
