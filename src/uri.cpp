
#include "uri.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

using namespace std;

namespace rj
{
    namespace net
    {
        uri::uri(const std::string &uri, const char *defaultScheme) : uri_(uri)
        {
            isValid_ = parse(uri, defaultScheme);
        }

        std::string uri::to_string() const
        {
            return uri_;
        }

        uri::operator std::string() const
        {
            return uri_;
        }

        bool uri::parse(const std::string &uri_s, const char *defaultScheme)
        {
#ifdef HAVE_LIBURIPARSER
            UriParserStateA state_;
            state_.uri = &uriParse_;
            return uriParseUriA(&state_, uri_s.c_str()) == URI_SUCCESS;
#else
            // do the manual implementation from stack overflow
            // with some mods for the port
            const string prot_end("://");
            string::const_iterator pos_i = search(uri_s.begin(), uri_s.end(), prot_end.begin(), prot_end.end());
            if (pos_i == uri_s.end()) {
                if (defaultScheme == NULL || !*defaultScheme) {
                    return false;
                } else {
                    scheme_ = defaultScheme;
                    pos_i = uri_s.begin();
                }
            } else {
                scheme_.reserve(distance(uri_s.begin(), pos_i));
                transform(uri_s.begin(), pos_i, back_inserter(scheme_), ptr_fun<int, int>(tolower));  // protocol is icase
                advance(pos_i, prot_end.length());
            }

            string::const_iterator user_i = find(pos_i, uri_s.end(), '@');
            string::const_iterator path_i;

            if (user_i != uri_s.end()) {
                string::const_iterator pwd_i = find(pos_i, user_i, ':');

                if (pwd_i != user_i) {
                    password_.assign(pwd_i, user_i);
                    user_.assign(pos_i, pwd_i);
                } else {
                    user_.assign(pos_i, user_i);
                }

                pos_i = user_i + 1;
            }

            path_i = find(pos_i, uri_s.end(), '/');

            string::const_iterator port_i = find(pos_i, path_i, ':');
            string::const_iterator host_end;
            if (port_i != uri_s.end()) {
                port_.assign(*port_i == ':' ? (port_i + 1) : port_i, path_i);
                host_end = port_i;
            } else {
                host_end = path_i;
            }
            host_.reserve(distance(pos_i, host_end));
            transform(pos_i, host_end, back_inserter(host_), ptr_fun<int, int>(tolower));  // host is icase
            string::const_iterator query_i = find(path_i, uri_s.end(), '?');
            path_.assign(*path_i == '/' ? (path_i + 1) : path_i, query_i);
            if (query_i != uri_s.end()) ++query_i;
            string::const_iterator frag_i = find(query_i, uri_s.end(), '#');
            query_.assign(query_i, frag_i);
            if (frag_i != uri_s.end()) {
                fragment_.assign(frag_i, uri_s.end());
            }
            return true;
#endif
        }

        uri::~uri()
        {
#ifdef HAVE_LIBURIPARSER
            uriFreeUriMembersA(&uriParse_);
#endif
        }

        bool uri::is_valid() const
        {
            return isValid_;
        }

        std::string uri::scheme() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.scheme);
#else
            return scheme_;
#endif
        }

        std::string uri::username() const
        {
#ifdef HAVE_LIBURIPARSER
            const char *userInfo = fromRange(uriParse_.userInfo);
            if (userInfo) {
                std::string temp(userInfo);
                return temp.substr(0, temp.find(':'));
            } else {
                return std::string();
            }
#else
            return user_;
#endif
        }

        std::string uri::password() const
        {
#ifdef HAVE_LIBURIPARSER
            const char *userInfo = fromRange(uriParse_.userInfo);
            if (userInfo) {
                std::string temp(userInfo);
                return temp.substr(temp.find(':') + 1);
            } else {
                return std::string();
            }
#else
            return password_;
#endif
        }

        std::string uri::host() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.hostText);
#else
            return host_;
#endif
        }
        std::string uri::port() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.portText);
#else
            return port_;
#endif
        }

        std::string uri::host_with_port() const
        {
            std::string hostname = host();

            if (!port().empty()) {
                return hostname + ":" + port();
            }
            return hostname;
        }

        std::string uri::path() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromList(uriParse_.pathHead, "/");
#else
            return path_;
#endif
        }
        std::string uri::query() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.query);
#else
            return query_;
#endif
        }
        std::string uri::fragment() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.fragment);
#else
            return fragment_;
#endif
        }
#ifdef HAVE_LIBURIPARSER
        std::string uri::fromRange(const UriTextRangeA &rng) const
        {
            if (rng.first && rng.afterLast) {
                return std::string(rng.first, rng.afterLast);
            } else {
                return std::string();
            }
        }

        std::string uri::fromList(UriPathSegmentA *xs, const std::string &delim) const
        {
            UriPathSegmentStructA *head(xs);
            std::string accum;

            while (head) {
                accum += delim + fromRange(head->text);
                head = head->next;
            }

            return accum;
        }
#endif
    }
}
