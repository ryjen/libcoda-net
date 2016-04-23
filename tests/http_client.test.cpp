#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#undef VERSION

#include <bandit/bandit.h>
#include <string>
#include <thread>
#include "http_client.h"
#include "polling_socket_server.h"
#include "socket_client.h"

using namespace bandit;

using namespace arg3::net;

using namespace arg3;

using namespace std;

class test_socket_factory : public socket_factory, public buffered_socket_listener, public enable_shared_from_this<test_socket_factory>
{
   private:
    string response_;

   public:
    socket_factory::socket_type create_socket(const server_type &server, SOCKET sock, const sockaddr_storage &addr)
    {
        auto socket = make_shared<socket_client>(sock, addr);

        socket->add_listener(shared_from_this());

        socket->start();

        return socket;
    }

    void set_response(const string &response)
    {
        response_ = response;
    }

    void on_connect(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} connected", sock->getIP()));
    }

    void on_close(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} closed", sock->getIP()));
    }

    void on_will_read(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} will read", sock->getIP()));
    }

    void on_did_read(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} did read", sock->getIP()));

        string line = sock->readln();

        string method = line.substr(0, line.find(' '));

        sock->write(method + ": " + response_);
    }

    void on_will_write(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} will write", sock->getIP()));
    }

    void on_did_write(const buffered_socket_listener::socket_type &sock)
    {
        // log::trace(format("{0} did write", sock->getIP()));

        sock->close();
    }
};

go_bandit([]() {

    std::shared_ptr<test_socket_factory> testFactory = std::make_shared<test_socket_factory>();

    socket_server testServer(testFactory);

    describe("an http client", [&]() {
        before_each([&testServer]() {
            try {
                testServer.start_in_background(9876);
            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
            }
        });

        after_each([&testServer]() { testServer.stop(); });

        it("can get", [&]() {
            http_client client("localhost:9876");

            testFactory->set_response("Hello, World!");

            try {
                client.get("test");

                Assert::That(client.response().payload(), Equals("GET: Hello, World!"));
            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
                throw e;
            }
        });
#if defined(HAVE_LIBSSL)
        it("is secure", []() {
            http_client client("google.com");

            client.set_secure(true);

            client.get("/settings/personalinfo");

            Assert::That(client.response().payload().empty(), Equals(false));
        });
#endif
        it("can post", []() {
            http_client client("localhost:9876");

            client.set_payload("Hello, World!");

            try {
                client.post("test");

                Assert::That(client.response().payload(), Equals("POST: Hello, World!"));
            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
                throw e;
            }
        });

        it("can read http response", []() {
            http_client client("www.arg3.com");

            try {
                client.get("/");

                auto response = client.response();

                Assert::That(response.payload().empty(), Equals(false));

                Assert::That(response.payload().find("<!DOCTYPE html>"), !Equals(string::npos));

            } catch (const exception &e) {
                std::cerr << typeid(e).name() << ": " << e.what() << std::endl;
                throw e;
            }
        });
    });

});
