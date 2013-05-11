#include <igloo/igloo.h>
#include "restclient.h"
#include "socketserver.h"
#include "bufferedsocket.h"
#include "../log/log.h"
#include "../format/format.h"
#include "../string/argument.h"
#include <string>
#include <thread>

using namespace igloo;

using namespace arg3::net;

using namespace arg3;

using namespace std;

class TestSocketListener : public BufferedSocketListener
{
private:
    string response_;

public:
    void setResponse(const string &response) {
        response_ = response;
    }

    void onConnect(BufferedSocket *sock)
    {
        log::trace(format("{0} connected", sock->getIP()));
    }

    void onClose(BufferedSocket *sock)
    {
        log::trace(format("{0} closed", sock->getIP()));
    }

    void onWillRead(BufferedSocket *sock)
    {
        log::trace(format("{0} will read", sock->getIP()));
    }

    void onDidRead(BufferedSocket *sock)
    {
        log::trace(format("{0} did read", sock->getIP()));

        argument line = sock->readLine();

        string method = line.next();

        sock->write(method + ": " + response_);
    }

    void onWillWrite(BufferedSocket *sock)
    {
        log::trace(format("{0} will write", sock->getIP()));
    }

    void onDidWrite(BufferedSocket *sock)
    {
        log::trace(format("{0} did write", sock->getIP()));

        sock->close();
    }
};

TestSocketListener listener;

class TestSocketFactory : public DefaultSocketFactory
{
public:
    BufferedSocket *createSocket(SOCKET sock, const sockaddr_in &addr)
    {
        BufferedSocket *socket = DefaultSocketFactory::createSocket(sock, addr);

        socket->addListener(&listener);

        return socket;
    }

};

TestSocketFactory testFactory;

SocketServer testServer(9876, &testFactory);

Context(arg3restclient)
{
    static void SetUpContext()
    {
        try {
            testServer.start();

            log::trace("Mock server started");
        }
        catch(const exception &e) {
            log::trace(e.what());
        }
    }

    static void TearDownContext()
    {
        testServer.stop();
    }

    Spec(testGet)
    {
        RESTClient client("localhost:9876", "1");

        listener.setResponse("Hello, World!");

        try {
            client.get("test");
            Assert::That(client.getResponse(), Equals("GET: Hello, World!"));
        }
        catch(const exception &e) {
            log::trace(e.what());
            throw e;
        }
    }

    Spec(testPost)
    {
        RESTClient client("localhost:9876", "1");

        client.setPayload("Hello, World!");

        try {
            client.post("test");
            Assert::That(client.getResponse(), Equals("POST: Hello, World!"));
        }
        catch(const exception &e) {
            log::trace(e.what());
            throw e;
        }
    }
};
