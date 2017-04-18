
#include <string>

#include <bandit/bandit.h>
#include "http/client.h"

using namespace bandit;
using namespace rj::net;

int main(int argc, char *argv[])
{
#ifdef CURL_FOUND
    http::client::set_request_type(http::curl::request);

    if (bandit::run(argc, argv)) {
        return EXIT_FAILURE;
    }
#endif

    http::client::set_request_type(http::socket::request);

    return bandit::run(argc, argv);
}