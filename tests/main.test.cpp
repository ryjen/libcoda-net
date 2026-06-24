
#include <string>

#include <bandit/bandit.h>
#include "socket.h"
#include "uri.h"
#include "http/client.h"

using namespace bandit;
using namespace coda::net;
using namespace snowhouse;

go_bandit([]() {
    describe("uri", []() {
        it("decodes complete percent escapes", []() {
            AssertThat(uri::decode("hello%20world"), Equals("hello world"));
        });

        it("preserves incomplete percent escapes", []() {
            AssertThat(uri::decode("%"), Equals("%"));
            AssertThat(uri::decode("%2"), Equals("%2"));
        });

        it("preserves invalid percent escapes", []() {
            AssertThat(uri::decode("%zz"), Equals("%zz"));
        });

        it("uses the default scheme when omitted", []() {
            uri parsed("localhost:9876/test");

            AssertThat(parsed.is_valid(), IsTrue());
            AssertThat(parsed.scheme(), Equals("http"));
            AssertThat(parsed.host(), Equals("localhost"));
            AssertThat(parsed.port(), Equals("9876"));
            AssertThat(parsed.path(), Equals("/test"));
        });
    });
});

int main(int argc, char *argv[])
{
    return bandit::run(argc, argv);
}
