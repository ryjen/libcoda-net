#ifndef RJ_NET_JEST_JESTER_H
#define RJ_NET_JEST_JESTER_H

#include <map>
#include <string>
#include "../client.h"

namespace rj
{
    namespace net
    {
        class jester
        {
           public:
            jester();

            jester &set_uri(const std::string &value);
            jester &set_method(http::method value);
            bool set_method(const std::string &value);

            jester &set_interactive(bool value);
            bool is_interactive() const;

            jester &append_data(const std::string &key, const std::string &value);
            jester &append_data(const std::pair<std::string, std::string> &value);

            jester &append_header(const std::string &key, const std::string &value);
            jester &append_header(const std::pair<std::string, std::string> &value);

            jester &set_content(const std::string &value);

            http::method method() const;
            rj::net::uri uri() const;

            void syntax(const char *bin) const;
            int parse_options(int argc, char *argv[]);

            bool validate() const;
            http::response execute(const std::string &path) const;

           private:
            http::method method_;
            std::map<std::string, std::string> data_;
            std::string content_;
            std::map<std::string, std::string> headers_;
            rj::net::uri uri_;
            bool interactive_;
        };
    }
}

#endif
