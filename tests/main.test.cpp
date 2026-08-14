
#include <string>

#include <bandit/bandit.h>
#include "socket.h"
#include "uri.h"
#include "http/client.h"

using namespace bandit;
using namespace coda::net;

int main(int argc, char *argv[])
{
    http::client::set_request_type(http::socket::request);
    return bandit::run(argc, argv);
}
