#include "uri.h"
#include <stdexcept>

namespace arg3
{
    namespace net
    {
        uri::uri(std::string uri) : uri_(uri)
        {
#ifdef HAVE_LIBURIPARSER
            UriParserStateA state_;
            state_.uri = &uriParse_;
            isValid_ = uriParseUriA(&state_, uri_.c_str()) == URI_SUCCESS;
#else
            isValid_ = false;
            throw std::runtime_error("uriparser library not enabled.");
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
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
        std::string uri::host() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.hostText);
#else
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
        std::string uri::port() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.portText);
#else
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
        std::string uri::path() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromList(uriParse_.pathHead, "/");
#else
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
        std::string uri::query() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.query);
#else
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
        std::string uri::fragment() const
        {
#ifdef HAVE_LIBURIPARSER
            return fromRange(uriParse_.fragment);
#else
            throw std::runtime_error("uriparser library not enabled.");
#endif
        }
#ifdef HAVE_LIBURIPARSER
        std::string uri::fromRange(const UriTextRangeA &rng) const
        {
            return std::string(rng.first, rng.afterLast);
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