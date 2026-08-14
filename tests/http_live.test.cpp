#include <string>

#include <bandit/bandit.h>
#include "http/client.h"

using namespace bandit;
using namespace coda::net;
using namespace coda;
using namespace std;
using namespace snowhouse;

go_bandit([]() {
    describe("an http client using live external services", []() {
#ifdef OPENSSL_FOUND
        it("can make a secure external request", []() {
            http::client client("https://www.httpvshttps.com");

            Assert::That(client.is_secure(), IsTrue());

            client.get();

            Assert::That(client.response().content().empty(), Equals(false));
        });
#endif

        it("can read an external http response", []() {
            http::client client("http://www.httpvshttps.com");

            client.get();

            auto response = client.response();

            Assert::That(response.content().empty(), Equals(false));
            Assert::That(response.content().find("<html"), !Equals(string::npos));
        });
    });
});
