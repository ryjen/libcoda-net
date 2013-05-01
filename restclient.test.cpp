#include <igloo/igloo.h>
#include "restclient.h"
#include "socketserver.h"
#include "bufferedsocket.h"
#include "../log/log.h"
#include "../format/format.h"
#include <string>
#include <thread>

using namespace igloo;

using namespace arg3::net;

using namespace arg3;

using namespace std;

class TestServerListener : public SocketServerListener
{
    void onConnect(BufferedSocket &sock)
    {
        log::trace(format("{0} connected", sock.getIP()));
    }

    void onClose(BufferedSocket &sock)
    {
        log::trace(format("{0} closed", sock.getIP()));
    }

    void onWillRead(BufferedSocket &sock)
    {
        log::trace(format("{0} will read", sock.getIP()));
    }

    void onDidRead(BufferedSocket &sock)
    {
        log::trace(format("{0} did read", sock.getIP()));

        sock.write(sock.getInput());
    }

    void onWillWrite(BufferedSocket &sock)
    {
        log::trace(format("{0} will write", sock.getIP()));
    }

    void onDidWrite(BufferedSocket &sock)
    {
        log::trace(format("{0} did write", sock.getIP()));

        sock.close();
    }
};

class TestServer : public SocketServer
{
public:
    TestServer() : SocketServer(9876)
    {
        addListener(&listener_);
    }
private:
    TestServerListener listener_;
};

TestServer testServer;

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

        try {
            client.get("test");
            log::trace(client.getResponse());
        }
        catch(const exception &e) {
            log::trace(e.what());
        }
    }
};
