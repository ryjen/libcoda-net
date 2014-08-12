#ifndef HAVE_LIBCURL

#include <bandit/bandit.h>
#include "http_client.h"
#include "socket_server.h"
#include "buffered_socket.h"
#include <string>
#include <thread>

using namespace bandit;

using namespace arg3::net;

using namespace arg3;

using namespace std;

class test_socket_factory : public socket_factory, public buffered_socket_listener
{

private:
    string response_;

public:
    std::shared_ptr<buffered_socket> create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr)
    {
        auto socket = make_shared<buffered_socket>(sock, addr);

        socket->add_listener(this);

        return socket;
    }

    void set_response(const string &response)
    {
        response_ = response;
    }

    void on_connect(buffered_socket *sock)
    {
        //log::trace(format("{0} connected", sock->getIP()));
    }

    void on_close(buffered_socket *sock)
    {
        //log::trace(format("{0} closed", sock->getIP()));
    }

    void on_will_read(buffered_socket *sock)
    {
        //log::trace(format("{0} will read", sock->getIP()));
    }

    void on_did_read(buffered_socket *sock)
    {
        //log::trace(format("{0} did read", sock->getIP()));

        string line = sock->readln();

        string method = line.substr(0, line.find(' '));

        sock->write(method + ": " + response_);
    }

    void on_will_write(buffered_socket *sock)
    {
        //log::trace(format("{0} will write", sock->getIP()));
    }

    void on_did_write(buffered_socket *sock)
    {
        //log::trace(format("{0} did write", sock->getIP()));

        sock->close();
    }
};

test_socket_factory testFactory;

socket_server testServer(9876, &testFactory);

thread serverThread;

go_bandit([]()
{

    describe("an http client", []()
    {
        before_each([]()
        {
            try
            {
                serverThread = testServer.start_thread();

                //log::trace("Mock server started");
            }
            catch (const exception &e)
            {
                std::cerr << e.what() << std::endl;
            }
        });

        after_each([]()
        {
            testServer.close();

            serverThread.join();
        });

        it("can get", []()
        {
            http_client client("localhost:9876");

            testFactory.set_response("Hello, World!");

            try
            {
                client.get("test");
                Assert::That(client.response(), Equals("GET: Hello, World!"));
            }
            catch (const exception &e)
            {
                std::cerr << e.what() << std::endl;
                throw e;
            }
        });

        it("is secure", []()
        {
            http_client client("google.com");

            client.set_secure(true);

            client.get("/settings/personalinfo");

            Assert::That(client.response().empty(), Equals(false));
        });

        it("can post", []()
        {
            http_client client("localhost:9876");

            client.set_payload("Hello, World!");

            try
            {
                client.post("test");
                Assert::That(client.response(), Equals("POST: Hello, World!"));
            }
            catch (const exception &e)
            {
                std::cerr << e.what() << std::endl;
                throw e;
            }
        });
    });

});

#endif
