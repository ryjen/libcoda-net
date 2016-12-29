
#include <bandit/bandit.h>
#include "http/client.h"

using namespace bandit;
using namespace rj::net;

int main(int argc, char *argv[])
{
#ifdef CURL_FOUND
    if (bandit::run(argc, argv)) {
        return EXIT_FAILURE;
    }
    http::client::set_implementation(http::curl::request);
#endif

    return bandit::run(argc, argv);
}