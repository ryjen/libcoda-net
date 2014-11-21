#include "uri.h"

namespace arg3
{
    namespace net
    {

        uri::uri(std::string uri)
            : uri_(uri)
        {
            UriParserStateA state_;
            state_.uri = &uriParse_;
            isValid_   = uriParseUriA(&state_, uri_.c_str()) == URI_SUCCESS;
        }

        uri::~uri()
        {
            uriFreeUriMembersA(&uriParse_);
        }

        bool uri::is_valid() const
        {
            return isValid_;
        }

        std::string uri::scheme()   const
        {
            return fromRange(uriParse_.scheme);
        }
        std::string uri::host()     const
        {
            return fromRange(uriParse_.hostText);
        }
        std::string uri::port()     const
        {
            return fromRange(uriParse_.portText);
        }
        std::string uri::path()     const
        {
            return fromList(uriParse_.pathHead, "/");
        }
        std::string uri::query()    const
        {
            return fromRange(uriParse_.query);
        }
        std::string uri::fragment() const
        {
            return fromRange(uriParse_.fragment);
        }

        std::string uri::fromRange(const UriTextRangeA &rng) const
        {
            return std::string(rng.first, rng.afterLast);
        }

        std::string uri::fromList(UriPathSegmentA *xs, const std::string &delim) const
        {
            UriPathSegmentStructA *head(xs);
            std::string accum;

            while (head)
            {
                accum += delim + fromRange(head->text);
                head = head->next;
            }

            return accum;
        }
    }
}