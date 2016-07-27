#ifndef RJ_NET_URI_H_
#define RJ_NET_URI_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_LIBURIPARSER
#include <uriparser/Uri.h>
#endif

#include <string>

namespace rj
{
    namespace net
    {
        class uri
        {
           public:
            uri(const std::string &uri, const char *defaultScheme = "http");

            ~uri();

            bool is_valid() const;

            std::string scheme() const;
            std::string username() const;
            std::string password() const;
            std::string host() const;
            std::string host_with_port() const;
            std::string port() const;
            std::string path() const;
            std::string query() const;
            std::string fragment() const;

            std::string to_string() const;
            operator std::string() const;

           private:
            std::string uri_;
            bool isValid_;
            bool parse(const std::string &value, const char *defaultScheme);
#ifdef HAVE_LIBURIPARSER
            UriUriA uriParse_;

            std::string fromRange(const UriTextRangeA &rng) const;

            std::string fromList(UriPathSegmentA *xs, const std::string &delim) const;
#else
            std::string scheme_, user_, password_, host_, port_, path_, query_, fragment_;
#endif
        };
    }
}

#endif
